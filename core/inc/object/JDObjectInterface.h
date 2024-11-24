#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "utilities/JDSerializable.h"
#include "utilities/JDUser.h"
#include "manager/JDObjectLocker.h"
#include "JDObjectID.h"
#include <memory>

#include "Json/JsonValue.h"
#include <string>
#include <QIcon>
#include <QColor>



namespace JsonDatabase
{
    
    
    
class JSON_DATABASE_EXPORT JDObjectInterface: protected Utilities::JDSerializable
{
        friend JDManager;
        friend Internal::JDManagerObjectManager;
        friend Utilities::JsonUtilities;
        friend Internal::JDObjectManager;

        friend class AutoObjectAddToRegistry;

    public:
        enum class Color
        {
            Default,
            Error,
            UnsavedChanges,
            Locked


        };

        JDObjectInterface();

        
        JDObjectInterface(const JDObjectInterface &other);
        virtual ~JDObjectInterface();

        JDObject deepClone() const;
        template<typename T>
        std::shared_ptr<T> deepClone() const;

        JDObject shallowClone() const;
        template<typename T>
        std::shared_ptr<T> shallowClone() const;
        

        
        static size_t getJsonIndexByID(const JsonArray& jsons, const JDObjectID::IDType& objID);
        static JDObjectID::IDType getIDFromJson(const JsonObject& obj);

        bool loadFrom(const JDObject& source);
        bool loadFrom(const JDObjectInterface* source);

        /**
         * @brief 
         * A Object is managed if it is added to a Database
		 * @return true if the objects in a database, otherwise false
         */
        bool isManaged() const;


        virtual const std::string& className() const = 0;
		virtual std::string getToolTip() const { return ""; }
		virtual std::string getDisplayName() const { return className(); }

        /**
         * @brief 
		 * Gets the object id of the object. If the object is unmanaged the id is empty.
		 * @return the object id
         */
        JDObjectIDptr getObjectID() const;

        /**
         * @brief 
         * The shallow ID is a backup id for the case the object is unmanaged but was copied from an managed object.
         *   This id has a weak bound to the object if it is managed.
         *  Always try to use
         *  JDObjectIDptr getObjectID() const;
         *  to get the id of the object. Only if it returns an empty pointer use
         *  const JDObjectID::IDType& getShallowObjectID() const;
		 * @return the shallow object id
         */
        const JDObjectID::IDType& getShallowObjectID() const;


        // Interface to the database

        /**
         * @brief 
		 * Gets the lock state of the object
         * @return true if the object is locked, otherwise false
         */
        bool isLocked() const;

        /**
         * @brief 
		 * Tries to lock the object
		 * @return true if the object was successfully locked, false otherwise
         */
        bool lock();

        /**
         * @brief 
		 * Tries to unlock the object
		 * @return true if the object was successfully unlocked, false otherwise
         */
        bool unlock();

        /**
         * @brief 
		 * Gets the user who locked the object if it is locked
		 * @param user parameter to store the user
		 * @return true if the object is locked by any user, otherwise false
         */
        bool getLockOwner(Utilities::JDUser &user) const;

        /**
         * @brief 
		 * Gets the lock data of the object if it is locked
         * @param data 
         * @return true if the object is locked, otherwise false
         */
        bool getLockData(Internal::JDObjectLocker::LockData& data) const;

        /**
         * @brief 
		 * Tries to save the object to the database,
         * this only works if the object is locked by this session
		 * @return true if the object was successfully saved, false otherwise
         */
        bool saveToDatabase();

        /**
         * @brief 
		 * Tries to save the object to the database asynchronously,
		 * this only works if the object is locked by this session
         */
        void saveToDatabaseAsync();

        /**
         * @brief 
		 * Tries to load the object from the database
		 * @return true if the object was successfully loaded, false otherwise
         */
        bool loadFromDatabase();

        /**
         * @brief 
		 * Tries to load the object from the database asynchronously
         */
        void loadFromDatabaseAsync();

		/**
		 * @brief 
         * Define a custom icon which is visible in the object list view
		 * @return 
		 */
		virtual QIcon getIcon() const { return QIcon(); }

        /**
         * @brief 
		 * Define a custom color which is used to color the object in the object list view
         * @return 
         */
        virtual QColor getColor() const { return QColor(0,0,0,0); }

    protected:
        /**
         * @brief 
		 * Compares the data of this object with the json data of another object
		 * @param obj with the data from another object
		 * @return true if the data is equal, otherwise false
         */
        bool equalData(const JsonObject& obj) const;

        /**
         * @brief 
		 * Loads the data from a json object to this object
		 * @param obj with the data to load
		 * @return true if the data was successfully loaded, false otherwise
         */
        bool loadInternal(const JsonObject& obj);

        /**
         * @brief 
		 * Saves the data of this object to a json object
		 * @param obj in which the data is saved
		 * @return true if the data was successfully saved, false otherwise
         */
        bool saveInternal(JsonObject& obj) const;
        bool getSaveData(JsonObject& obj) const;


    class JSON_DATABASE_EXPORT AutoObjectAddToRegistry
    {
    public:
        AutoObjectAddToRegistry(JDObject obj);
        int addToRegistry(JDObject obj);
    };

    private:

        void setManager(Internal::JDObjectManager* manager);
        Internal::JDObjectManager* getManager() const;

        virtual JDObjectInterface* deepClone_internal() const = 0;
        virtual JDObjectInterface* shallowClone_internal() const = 0;




        Internal::JDObjectManager* m_manager;

        /*
            The shallow ID is a backup id for the case the object is unmanaged but was copied from an managed object.
            This id has a weak bound to the object if it is managed.
            Always try to use 
            JDObjectIDptr getObjectID() const;
            to get the id of the object. Only if it returns an empty pointer use 
            const JDObjectID::IDType& getShallowObjectID() const;
        */
        JDObjectID::IDType m_shallowID;

    public:

        static const std::string s_tag_objID;
        static const std::string s_tag_className;
        static const std::string s_tag_data;
    private:
        
};

Q_DECLARE_METATYPE(std::vector<JDObject>);
Q_DECLARE_METATYPE(JDObject);

template<typename T>
std::shared_ptr<T> JDObjectInterface::deepClone() const
{
    JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    T* cloned = dynamic_cast<T*>(deepClone_internal());
    std::shared_ptr<T> clone(cloned);
    return clone;
}
template<typename T>
std::shared_ptr<T> JDObjectInterface::shallowClone() const
{
    JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    T* cloned = dynamic_cast<T*>(shallowClone_internal());
    std::shared_ptr<T> clone(cloned);
    return clone;
}



/**
 * \macro JD_OBJECT(classNameVal)
 *        Implements the virtual clone() function of the JDObjectInterface
 *        Implements the virtual className() function of the JDObjectInterface
 */
#define JD_OBJECT(classNameVal) \
    public: \
    JD_OBJECT_DECL_CONSTRUCTOR_COPY(classNameVal) \
    JD_OBJECT_DECL_CLONE(classNameVal) \
    JD_OBJECT_DECL_CLASSNAME(classNameVal) \
    JD_OBJECT_DECL_AUTOREGISTRY(classNameVal) 



#define JD_OBJECT_DECL_CONSTRUCTOR_COPY(classNameVal) \
    classNameVal(const classNameVal &other); 


#define JD_OBJECT_DECL_CLONE(classNameVal) \
    classNameVal* deepClone_internal() const override; \
    classNameVal* shallowClone_internal() const override; \

#define JD_OBJECT_DECL_CLASSNAME(classNameVal) \
    const std::string &className() const override; 

#define JD_OBJECT_DECL_AUTOREGISTRY(classNameVal) \
    private: \
    static AutoObjectAddToRegistry s__autoObjectRegistrator;


// Implementations 

#define JD_OBJECT_IMPL_CONSTRUCTOR_COPY(classNameVal) \
    classNameVal::classNameVal(const classNameVal &other) \
        : JDObjectInterface(other) \
    {} 



#define JD_OBJECT_IMPL_CLONE(classNameVal) \
    classNameVal* classNameVal::deepClone_internal() const \
    { \
        JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_5); \
        classNameVal* c = new classNameVal(*this); \
        return c; \
    } \
    classNameVal* classNameVal::shallowClone_internal() const \
    { \
        JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_5); \
        classNameVal* c = new classNameVal(); \
        return c; \
    } \

#define JD_OBJECT_IMPL_CLASSNAME(classNameVal) \
    const std::string &classNameVal::className() const \
    { \
        static std::string name = #classNameVal; \
        return name; \
    } 


#define JD_OBJECT_IMPL_AUTOREGISTRY(classNameVal) \
    classNameVal::AutoObjectAddToRegistry classNameVal::s__autoObjectRegistrator(std::shared_ptr<classNameVal>(new classNameVal()));


#define JD_OBJECT_IMPL(classNameVal) \
    JD_OBJECT_IMPL_CLONE(classNameVal) \
    JD_OBJECT_IMPL_CLASSNAME(classNameVal) \
    JD_OBJECT_IMPL_AUTOREGISTRY(classNameVal) 


}
