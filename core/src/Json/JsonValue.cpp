#include "Json/JsonValue.h"
#if JD_ACTIVE_JSON == JD_JSON_INTERNAL || JD_ACTIVE_JSON == JD_JSON_GLAZE
#include "Json/JsonSerializer.h"
#include <QDebug>

#endif


namespace JsonDatabase
{
#if JD_ACTIVE_JSON == JD_JSON_INTERNAL
    std::string JsonValue::serialize() const
    {
        JsonSerializer serializer;
        return serializer.serializeValue(*this);
    }

    std::ostream& operator<<(std::ostream& os, const JsonValue& json)
    {
        os << json.toString();
        return os;
    }

    // Overloading << operator for qDebug()
    QDebug operator<<(QDebug debug, const JsonValue& json)
    {
        QDebugStateSaver saver(debug);
        debug << json.toString().c_str();
        return debug;
    }
#endif
}