#include "AbstractConstraintHeader.hpp"
const QFont ConstraintHeader::m_font("Ubuntu", 10, QFont::Bold);
void ConstraintHeader::setWidth(double width)
{
    prepareGeometryChange();
    m_width = width;
}

void ConstraintHeader::setText(const QString &text)
{
    m_text = text;
    update();
}
