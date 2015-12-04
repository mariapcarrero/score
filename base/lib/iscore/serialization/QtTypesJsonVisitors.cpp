#include <boost/none_t.hpp>
#include <boost/optional/optional.hpp>
#include <QJsonArray>
#include <QJsonValue>
#include <QPoint>

#include "DataStreamVisitor.hpp"
#include "JSONValueVisitor.hpp"

template <typename T> class Reader;
template <typename T> class Writer;

// TODO RENAME FILE
template<>
void Visitor<Reader<JSONValue>>::readFrom(const QPointF& pt)
{
    QJsonArray arr;
    arr.append(pt.x());
    arr.append(pt.y());

    val = arr;
}

template<>
void Visitor<Writer<JSONValue>>::writeTo(QPointF& pt)
{
    auto arr = val.toArray();
    pt.setX(arr[0].toDouble());
    pt.setY(arr[1].toDouble());
}



template<>
ISCORE_LIB_BASE_EXPORT void Visitor<Reader<DataStream>>::readFrom(const boost::optional<double>& obj)
{
    m_stream << bool (obj);

    if(obj)
    {
        m_stream << get(obj);
    }
}

template<>
ISCORE_LIB_BASE_EXPORT void Visitor<Writer<DataStream>>::writeTo(boost::optional<double>& obj)
{
    bool b {};
    m_stream >> b;

    if(b)
    {
        double val;
        m_stream >> val;

        obj = val;
    }
    else
    {
        obj = boost::none_t {};
    }
}

