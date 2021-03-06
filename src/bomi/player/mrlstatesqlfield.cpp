#include "mrlstatesqlfield.hpp"
#include "mrl.hpp"
#include "misc/json.hpp"
#include "enum/enums.hpp"
#include "video/videocolor.hpp"
#include "audio/audioequalizer.hpp"
#include "subtitle/submisc.hpp"

template<class T>
SIA _Is(int type) -> bool { return qMetaTypeId<T>() == type; }

template<class T>
static auto _ValueToSqlDataUsingJson(const QVariant &value) -> QVariant
    { return _JsonToString(json_io<T>()->toJson(value.value<T>())); }

template<class T>
static auto _SqlDataToValueUsingJson(const QVariant &sqlData) -> QVariant
{
    T t;
    if (json_io<T>()->fromJson(t, _JsonFromString(sqlData.toString())))
        return QVariant::fromValue<T>(t);
    return QVariant();
}

MrlStateSqlField::MrlStateSqlField(const QMetaProperty &property,
                                   const QVariant &def) noexcept
    : m_property(property)
    , m_sqlType(u"INTEGER"_q)
    , m_defaultValue(def)
{
    static const auto passthrough = [] (const QVariant &var) { return var; };
    m_d2v = m_v2d = std::cref(passthrough);
    switch (m_property.userType()) {
    case QMetaType::Int:         case QMetaType::UInt:
    case QMetaType::LongLong:    case QMetaType::ULongLong:
        break;
    case QMetaType::Double:
        m_sqlType = u"REAL"_q;
        break;
    case QMetaType::QString:
        m_sqlType = u"TEXT"_q;
        break;
    case QMetaType::Bool:
        m_v2d = [] (const QVariant &value)
            { return QVariant((int)value.toBool()); };
        m_d2v = [] (const QVariant &sqlData)
            { return QVariant((bool)sqlData.toInt()); };
        break;
    case QMetaType::QDateTime:
        m_v2d = [] (const QVariant &value)
            { return QVariant(value.toDateTime().toMSecsSinceEpoch()); };
        m_d2v = [] (const QVariant &sqlData)
            { return QDateTime::fromMSecsSinceEpoch(sqlData.toLongLong()); };
        break;
#define JSON_CASE_QT(type) \
    {case QMetaType::type: \
        m_v2d = _ValueToSqlDataUsingJson<type>; \
        m_d2v = _SqlDataToValueUsingJson<type>; \
        break;}
    JSON_CASE_QT(QPoint);    JSON_CASE_QT(QPointF);
    JSON_CASE_QT(QSize);     JSON_CASE_QT(QSizeF);
#undef JSON_CASE
    default: {
        const auto type = m_property.userType();
        const auto enumConv = _EnumNameVariantConverter(type);
        if (!enumConv.isNull()) {
            m_sqlType = u"TEXT"_q;
            const auto n2v = enumConv.nameToVariant;
            const auto v2n = enumConv.variantToName;
            m_v2d = [v2n] (const QVariant &value)
                { return QVariant(v2n(value)); };
            m_d2v = [n2v] (const QVariant &sqlData)
                { return n2v(sqlData.toString()); };
            break;
        }
#define JSON_CASE(T) \
        if (_Is<T>(type)) { \
            m_sqlType = u"TEXT"_q; \
            m_v2d = _ValueToSqlDataUsingJson<T>; \
            m_d2v = _SqlDataToValueUsingJson<T>; \
            break;\
        }
        JSON_CASE(VideoColor);
        JSON_CASE(SubtitleStateInfo);
        JSON_CASE(AudioEqualizer);
        if (_Is<Mrl>(type)) {
            m_sqlType = u"TEXT PRIMARY KEY NOT NULL"_q;
            m_v2d = [] (const QVariant &value)
                { return QVariant(value.value<Mrl>().toString()); };
            m_d2v = [] (const QVariant &sqlData) {
                if (sqlData.isNull() || !sqlData.isValid())
                    return QVariant();
                return QVariant::fromValue(Mrl::fromString(sqlData.toString()));
            };
            break;
        }
        Q_ASSERT(false);
    }}
}

/******************************************************************************/

auto MrlStateSqlFieldList::clear() -> void
{
    for (auto &q : m_queries)
        q.clear();
    m_fields.clear();
    m_where = MrlStateSqlField();
}

auto MrlStateSqlFieldList::prepareInsert(const QString &table) -> QString
{
    auto &insert = m_queries[Insert];
    insert.clear();
    if (m_fields.isEmpty())
        return QString();
    const auto phs = _ToStringList(m_fields, [&] (const MrlStateSqlField &) {
        return QString('?'_q);
    }).join(','_q);
    const auto cols = _ToStringList(m_fields, [&] (const MrlStateSqlField &f) {
        return QString::fromLatin1(f.property().name());
    }).join(','_q);
    insert = u"INSERT OR REPLACE INTO %1 (%2) VALUES (%3)"_q
            .arg(table).arg(cols).arg(phs);
    return insert;
}

auto MrlStateSqlFieldList::field(const QString &name) const -> Field
{
    for (auto &field : m_fields) {
        if (!name.compare(_L(field.property().name())))
            return field;
    }
    return MrlStateSqlField();
}

auto MrlStateSqlFieldList::prepareSelect(const QString &table,
                                         const Field &where) -> QString
{
    auto &select = m_queries[Select];
    select.clear();
    if (m_fields.isEmpty() || !where.isValid())
        return QString();
    m_where = where;
    const auto columns = _ToStringList(m_fields,
                                       [&] (const MrlStateSqlField &field) {
        return QString::fromLatin1(field.property().name());
    }).join(','_q);
    select = u"SELECT %1 FROM %2 WHERE %3 = ?"_q
            .arg(columns).arg(table).arg(_L(m_where.property().name()));
    return select;
}

auto MrlStateSqlFieldList::select(QSqlQuery &query, QObject *object,
                                  const QVariant &where) const -> bool
{
    auto &select = m_queries[Select];
    if (select.isEmpty())
        return false;
    if (!query.prepare(select))
        return false;
    query.addBindValue(m_where.sqlData(where));
    if (!query.exec() || !query.next())
        return false;
    const auto record = query.record();
    for (int i = 0; i < m_fields.size(); ++i) {
        Q_ASSERT(_L(m_fields[i].property().name()) == record.fieldName(i));
        m_fields[i].exportTo(object, record.value(i));
    }
    return true;
}

auto MrlStateSqlFieldList::insert(QSqlQuery &query, const QObject *o) -> bool
{
    if (!isInsertPrepared())
        return false;
    if (!query.prepare(m_queries[Insert]))
        return false;
    for (int i=0; i<m_fields.size(); ++i)
        query.bindValue(i, m_fields[i].sqlData(o));
    return query.exec();
}
