#include "MainWindow.h"
#include <qdebug.h>
#include <QCloseEvent>

#ifndef JD_PROFILING
#define EASY_BLOCK(name, color)
#define EASY_END_BLOCK
#define EASY_FUNCTION(color)
#define EASY_THREAD(name)
#endif

#define DEBUG qDebug() << m_manager->getUser().c_str() << "::" << __FUNCTION__ << ": "

MainWindow::MainWindow(const std::string& user, QWidget *parent)
	: QWidget(parent)
	, m_manager(nullptr)
{
	ui.setupUi(this);

	m_manager = new JDManager("asyncDatabase", "Person", "sessionID", user);

	m_uiPersonEditor = new UIPerson(ui.editor_frame);
	connect(m_uiPersonEditor, &UIPerson::savePerson, this, &MainWindow::onPersonSave);
	//JDManager::startProfiler();

	m_manager->loadObjectsAsync();
	//m_manager->addObject(createPersons());
	//m_manager->saveObjects();

    m_manager->getSignals().connect_databaseFileChanged_slot(this, &MainWindow::onDatabaseFileChanged);
    m_manager->getSignals().connect_lockedObjectsChanged_slot(this, &MainWindow::onLockedObjectsChanged);
    m_manager->getSignals().connect_objectRemovedFromDatabase_slot(this, &MainWindow::onObjectRemovedFromDatabase);
    m_manager->getSignals().connect_objectAddedToDatabase_slot(this, &MainWindow::onObjectAddedToDatabase);
    m_manager->getSignals().connect_objectChangedFromDatabase_slot(this, &MainWindow::onObjectChangedFromDatabase);
    m_manager->getSignals().connect_objectOverrideChangeFromDatabase_slot(this, &MainWindow::onObjectOverrideChangeFromDatabase);
    m_manager->getSignals().connect_databaseOutdated_slot(this, &MainWindow::onDatabaseOutdated);


    m_manager->getSignals().connect_onLoadObjectsDone_slot(this, &MainWindow::onLoadAllDone);
	m_manager->getSignals().connect_onLoadObjectDone_slot(this, &MainWindow::onLoadIndividualDone);
    m_manager->getSignals().connect_onSaveObjectsDone_slot(this, &MainWindow::onSaveAllDone);
    m_manager->getSignals().connect_onSaveObjectDone_slot(this, &MainWindow::onSaveIndividualDone);



	//m_manager->getSignals().disconnect_onLoadObjectDone_slot(this, &MainWindow::onLoadIndividualDone);
	connect(&m_timer, &QTimer::timeout, this, &MainWindow::onTimerFinished);
	m_timer.start(10);
}

MainWindow::~MainWindow()
{
	m_timer.stop();
	delete m_manager;
	m_manager = nullptr;
	//JDManager::stopProfiler(m_manager->getUser()+"_asyncProfile.prof");
}

void MainWindow::onTimerFinished()
{
	EASY_FUNCTION(profiler::colors::Amber);
	m_manager->update();
}

void MainWindow::on_loadDatabase_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG;
	m_manager->loadObjectsAsync();
	onTimerFinished();
}
void MainWindow::on_saveDatabase_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG;
	m_manager->saveObjectsAsync();
	onTimerFinished();
}
void MainWindow::on_addObject_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	QString id = ui.id_lineEdit->text();
	if (id.isEmpty())
	{
		DEBUG << "id is empty";
		return;
	}
	Person* p = new Person(id.toStdString(), "Samuel", "Richards", "Male", "30", "s.richards@randatmail.com", "666-1856-78", "Upper secondary", "Mechanic", "1", "2127", "Single", "4");
	DEBUG <<p->getObjectID().c_str();
	if (!m_manager->addObject(p))
	{
		DEBUG << "Can't add object to database";
	}
}
void MainWindow::on_deleteObject_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	
	JDObjectInterface* p = getSelectedObject();
	if (p)
	{
		DEBUG << p->getObjectID().c_str();
		m_manager->removeObject(p);
		delete p;
	}
	else
	{
		DEBUG << "Object not found";
	}
}
void MainWindow::on_editObject_pushButton_clicked()
{
	bool editMode = false;
	Person* p = getSelectedPerson();
	/*if (m_manager->lockObject(p))
	{
		editMode = true;
	}
	else
	{*/
		int lastError;
		if (m_manager->isObjectLockedByOther(p, lastError))
		{
			editMode = false;
		}
		else
		{
			editMode = true;
		}
		if (lastError != JsonDatabase::Internal::JDObjectLocker::Error::none)
			editMode = false;
	//}
	m_uiPersonEditor->setPerson(getSelectedPerson(), editMode);
}

void MainWindow::on_lockObject_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);

	JDObjectInterface *obj = getSelectedObject();
	if (m_manager->lockObject(obj))
	{
		DEBUG << "locked: " << obj->getObjectID().c_str();
	}
	else
	{
		if(obj)
			DEBUG << "Can't lock: " << obj->getObjectID().c_str();
		else
			DEBUG << "Can't lock object, nullptr";
	}
}
void MainWindow::on_unlockObject_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);

	JDObjectInterface* obj = getSelectedObject();
	if (m_manager->unlockObject(obj))
	{
		DEBUG << "unlocked: " << obj->getObjectID().c_str();
	}
	else
	{
		if (obj)
			DEBUG << "Can't unlock: " << obj->getObjectID().c_str();
		else
			DEBUG << "Can't unlock object, nullptr";
	}
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG;
	event->accept();
	emit closeWindow();
}


JDObjectInterface* MainWindow::getSelectedObject()
{
	return m_manager->getObject(ui.id_lineEdit->text().toStdString());
}
Person* MainWindow::getSelectedPerson()
{
	return dynamic_cast<Person*>(getSelectedObject());
}
void MainWindow::onDatabaseFileChanged()
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG;
	m_manager->loadObjectsAsync();
	onTimerFinished();
}
void MainWindow::onLockedObjectsChanged()
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG;
	std::vector<JsonDatabase::Internal::JDObjectLocker::LockData> locked;
	m_manager->getLockedObjects(locked);
	std::string text;
	for (auto& id : locked)
	{
		Person *p = m_manager->getObject<Person>(id.objectID);
		if (p)
		{
			text += "\"" + id.objectID + "\" object locked by \"" + id.owner + "\"\n";
		}
		else
			text += "\"" + id.objectID + "\" object not found\n";
	}
	ui.lockedObjects_label->setText(QString::fromStdString(text));
}
void MainWindow::onObjectRemovedFromDatabase(const JsonDatabase::JDObjectContainer& removed)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG;
	for (auto& obj : removed)
	{
		DEBUG << "  " << obj->getObjectID().c_str();
		delete obj;
	}
}
void MainWindow::onObjectAddedToDatabase(const JsonDatabase::JDObjectContainer& added)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG;
	for (auto& obj : added)
	{
		DEBUG << "  " << obj->getObjectID().c_str();
	}
}
void MainWindow::onObjectChangedFromDatabase(const std::vector<JsonDatabase::JDObjectPair>& changedPairs)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG;
	for (auto& obj : changedPairs)
	{
		DEBUG << "  " << obj.first->getObjectID().c_str();
	}
}
void MainWindow::onObjectOverrideChangeFromDatabase(const JsonDatabase::JDObjectContainer& overwritten)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG;
	for (auto& obj : overwritten)
	{
		DEBUG << "  " << obj->getObjectID().c_str();
	}
}
void MainWindow::onDatabaseOutdated()
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG;
	m_manager->loadObjectsAsync();
}



void MainWindow::onSaveAllDone(bool success)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG << "success: "<<(success?"true":"false");
}
void MainWindow::onSaveIndividualDone(bool success, JDObjectInterface* obj)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG  << obj->getObjectID().c_str() << " "<<(success ? "true" : "false");;
}
void MainWindow::onLoadAllDone(bool success)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG  << (success ? "true" : "false");;
}
void MainWindow::onLoadIndividualDone(bool success, JDObjectInterface* obj)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG << obj->getObjectID().c_str() <<" "<< (success ? "true" : "false");;
}

void MainWindow::onPersonSave(Person* person)
{
	if (person)
		m_manager->saveObjectAsync(person);
}