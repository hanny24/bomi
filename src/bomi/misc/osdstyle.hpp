#ifndef SUBTITLESTYLE_H
#define SUBTITLESTYLE_H

struct OsdStyle {
    struct Font {
        Font() { qfont.setPixelSize(height()); }
        auto family() const -> QString { return qfont.family(); }
        auto bold() const -> bool { return qfont.bold(); }
        auto italic() const -> bool { return qfont.italic(); }
        auto underline() const -> bool { return qfont.underline(); }
        auto strikeOut() const -> bool { return qfont.strikeOut(); }
        auto setFamily(const QString &family) -> void
            { qfont.setFamily(family); }
        auto setBold(bool bold) -> void { qfont.setBold(bold); }
        auto setItalic(bool italic) -> void { qfont.setItalic(italic); }
        auto setUnderline(bool underline) -> void
            { qfont.setUnderline(underline); }
        auto setStrikeOut(bool strikeOut) -> void
            { qfont.setStrikeOut(strikeOut); }
        const QFont &font() const { return qfont; }
        static constexpr auto height() -> int { return 20; }
        auto weight() const -> int { return qfont.weight(); }
        QColor color = { Qt::white };
        double size = 0.05;
        QFont qfont;
    };
    struct BBox {
        bool enabled = false;
        QColor color = {0, 0, 0, 127};
        QPointF padding = {0.3, 0.1};
    };
    struct Shadow {
        bool enabled = true, blur = true;
        QColor color = {0, 0, 0, 127};
        QPointF offset = {0.1, 0.1};
    };
    struct Outline {
        QColor color = {Qt::black};
        double width = 0.05;
        bool enabled = true;
    };
    struct Spacing {
        double line = 0, paragraph = 0;
    };
    QTextOption::WrapMode wrapMode = QTextOption::WrapAtWordBoundaryOrAnywhere;
    Shadow shadow;
    Outline outline;
    Font font;
    Spacing spacing;
    BBox bbox;
    auto toJson() const -> QJsonObject;
    auto setFromJson(const QJsonObject &json) -> bool;
};

Q_DECLARE_METATYPE(OsdStyle);

/******************************************************************************/

class OsdStyleWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(OsdStyle value READ value WRITE setValue)
public:
    OsdStyleWidget(QWidget *parent = nullptr);
    ~OsdStyleWidget();
    auto setTextAlphaChannel(bool on) -> void;
    auto setShadowVsible(bool visible) -> void;
    auto setOutlineVisible(bool visible) -> void;
    auto setSpacingVisible(bool visible) -> void;
    auto setBBoxVisible(bool visible) -> void;
    auto setValue(const OsdStyle &v) -> void;
    auto value() const -> OsdStyle;
private:
    struct Data;
    Data *d;

};

#endif // SUBTITLESTYLE_H
