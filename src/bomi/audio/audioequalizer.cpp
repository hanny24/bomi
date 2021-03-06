#include "audioequalizer.hpp"

std::array<double, 10> AudioEqualizer::s_freqs = {
    31.25,
    62.5,
    125,
    250,
    500,
    1000,
    2000,
    4000,
    8000,
    16000
};

// presets are copied from VLC

auto AudioEqualizer::prepare(Preset preset) -> std::initializer_list<double>
{
    switch (preset) {
    case Flat:
        return { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    case Classic:
        return { 0, 0, 0, 0, 0, 0, -7.2, -7.2, -7.2, -9.6 };
    case Club:
        return { 0, 0, 8, 5.6, 5.6, 5.6, 3.2, 0, 0, 0 };
    case Dance:
        return { 9.6, 7.2, 2.4, 0, 0, -5.6, -7.2, -7.2, 0, 0 };
    case FullBass:
        return { -8, 9.6, 9.6, 5.6, 1.6, -4, -8, -10.4, -11.2, -11.2 };
    case FullBassTreble:
        return { 7.2, 5.6, 0, -7.2, -4.8, 1.6, 8, 11.2, 12, 12 };
    case FullTreble:
        return { -9.6, -9.6, -9.6, -4, 2.4, 11.2, 16, 16, 16, 16.8 };
    case Headphones:
        return { 4.8, 11.2, 5.6, -3.2, -2.4, 1.6, 4.8, 9.6, 12.8, 14.4 };
    case LargeHall:
        return { 10.4, 10.4, 5.6, 5.6, 0, -4.8, -4.8, -4.8, 0, 0 };
    case Live:
        return { -4.8, 0, 4, 5.6, 5.6, 5.6, 4, 2.4, 2.4, 2.4 };
    case Party:
        return { 7.2, 7.2, 0, 0, 0, 0, 0, 0, 7.2, 7.2 };
    case Pop:
        return { -1.6, 4.8, 7.2, 8, 5.6, 0, -2.4, -2.4, -1.6, -1.6 };
    case Reggae:
        return { 0, 0, 0, -5.6, 0, 6.4, 6.4, 0, 0, 0 };
    case Rock:
        return { 8, 4.8, -5.6, -8, -3.2, 4, 8.8, 11.2, 11.2, 11.2 };
    case Ska:
        return { -2.4, -4.8, -4, 0, 4, 5.6, 8.8, 9.6, 11.2, 9.6 };
    case Soft:
        return { 4.8, 1.6, 0, -2.4, 0, 4, 8, 9.6, 11.2, 12 };
    case SoftRock:
        return { 4, 4, 2.4, 0, -4, -5.6, -3.2, 0, 2.4, 8.8 };
    case Techno:
        return { 8, 5.6, 0, -5.6, -4.8, 0, 8, 9.6, 9.6, 8.8 };
    default:
        return {};
    }
}

auto AudioEqualizer::name(Preset preset) -> QString
{
    switch (preset) {
    case Flat:          return tr("Flat");
    case Classic:       return tr("Classic");
    case Club:          return tr("Club");
    case Dance:         return tr("Dance");
    case FullBass:      return tr("Full bass");
    case FullBassTreble:return tr("Full bass treble");
    case FullTreble:    return tr("Full treble");
    case Headphones:    return tr("Headphones");
    case LargeHall:     return tr("Large hall");
    case Live:          return tr("Live");
    case Party:         return tr("Party");
    case Pop:           return tr("Pop");
    case Reggae:        return tr("Reggae");
    case Rock:          return tr("Rock");
    case Ska:           return tr("Ska");
    case Soft:          return tr("Soft");
    case SoftRock:      return tr("Soft rock");
    case Techno:        return tr("Techno");
    default:            return QString();
    }
}

auto AudioEqualizer::toJson() const -> QJsonObject
{
    QJsonArray json;
    for (auto &db : m_dbs)
        json.push_back(db);
    QJsonObject obj;
    obj.insert(u"dbs"_q, json);
    return obj;
}

auto AudioEqualizer::fromJson(const QJsonObject &json) -> AudioEqualizer
{
    AudioEqualizer eq; eq.setFromJson(json); return eq;
}

auto AudioEqualizer::setFromJson(const QJsonObject &json) -> bool
{
    auto array = json.value(u"dbs"_q).toArray();
    if (array.size() != size())
        return false;
    for (int i = 0; i < size(); ++i)
        m_dbs[i] = array[i].toDouble();
    return true;
}
