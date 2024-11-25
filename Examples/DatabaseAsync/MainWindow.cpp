#include "MainWindow.h"
#include <qdebug.h>
#include <QCloseEvent>
#include <QFileDialog>

#ifndef JD_PROFILING
#define EASY_BLOCK(name, color)
#define EASY_END_BLOCK
#define EASY_FUNCTION(color)
#define EASY_THREAD(name)
#endif

#define DEBUG_SIMPLE std::cout
#define DEBUG DEBUG_SIMPLE << m_manager->getUser().getName().c_str() << "::" << __FUNCTION__ << ": "


Log::LogObject logger("main");

MainWindow::MainWindow(const std::string& user, QWidget *parent)
	: QWidget(parent)
	, m_manager(nullptr)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromStdString(user));

	//if(m_manager)
	//	delete m_manager;

	m_manager = new JDManager;
	m_userListWidget = new UI::JDUserListWidget();
	ui.userList_frame->layout()->addWidget(m_userListWidget);
	m_userListWidget->setBaseSize(200, 200);
	m_userListWidget->show();
	m_objectListWidget = new UI::JDObjectListWidget(m_manager);
	ui.objectList_frame->layout()->addWidget(m_objectListWidget);
	m_objectListWidget->setBaseSize(200, 200);
	m_objectListWidget->show();
	connect(m_objectListWidget, &UI::JDObjectListWidget::objectClicked, this, &MainWindow::onObjectClicked);
	m_lockDataWidget = new UI::JDObjectLockDataWidget();
	ui.lockData_frame->layout()->addWidget(m_lockDataWidget);
	m_lockDataWidget->hide();


	Log::UI::QConsoleView* console = new Log::UI::QConsoleView();
	//Log::UI::QTreeConsoleView* console = new Log::UI::QTreeConsoleView();
	console->show();
	ui.console_frame->layout()->addWidget(console);

	m_manager->setup("asyncDatabase", "Person", user);

	m_uiPersonEditor = new UIPerson(ui.editor_frame);
	connect(m_uiPersonEditor, &UIPerson::savePerson, this, &MainWindow::onPersonSave);
	//JDManager::startProfiler();

	m_manager->loadObjectsAsync();
	//m_manager->addObject(createPersons());
	//m_manager->saveObjects();

	

    //m_manager->getSignals().connect_databaseFileChanged_slot(this, &MainWindow::onDatabaseFileChanged);
    //m_manager->getSignals().connect_lockedObjectsChanged_slot(this, &MainWindow::onLockedObjectsChanged);
    //m_manager->getSignals().connect_objectRemovedFromDatabase_slot(this, &MainWindow::onObjectRemovedFromDatabase);
    //m_manager->getSignals().connect_objectAddedToDatabase_slot(this, &MainWindow::onObjectAddedToDatabase);
    //m_manager->getSignals().connect_objectChangedFromDatabase_slot(this, &MainWindow::onObjectChangedFromDatabase);
    //m_manager->getSignals().connect_objectOverrideChangeFromDatabase_slot(this, &MainWindow::onObjectOverrideChangeFromDatabase);
    //m_manager->getSignals().connect_databaseOutdated_slot(this, &MainWindow::onDatabaseOutdated);
	//
	//
    //m_manager->getSignals().connect_onStartAsyncWork_slot(this, &MainWindow::onAsyncWorkStarted);
    //m_manager->getSignals().connect_onEndAsyncWork_slot(this, &MainWindow::onAsyncWorkFinished);
    //m_manager->getSignals().connect_onLoadObjectsDone_slot(this, &MainWindow::onLoadAllDone);
	//m_manager->getSignals().connect_onLoadObjectDone_slot(this, &MainWindow::onLoadIndividualDone);
    //m_manager->getSignals().connect_onSaveObjectsDone_slot(this, &MainWindow::onSaveAllDone);
    //m_manager->getSignals().connect_onSaveObjectDone_slot(this, &MainWindow::onSaveIndividualDone);



	connect(m_manager, &JDManager::databaseFileChanged, this, &MainWindow::onDatabaseFileChanged);
	connect(m_manager, &JDManager::lockedObjectsChanged, this, &MainWindow::onLockedObjectsChanged);
	connect(m_manager, &JDManager::objectRemoved, this, &MainWindow::onObjectRemovedFromDatabase);
	connect(m_manager, &JDManager::objectAdded, this, &MainWindow::onObjectAddedToDatabase);
	//connect(m_manager, &JDManager::objectChangedFromDatabase, this, &MainWindow::onObjectChangedFromDatabase);
	connect(m_manager, &JDManager::objectChanged, this, &MainWindow::onObjectChanged);
	connect(m_manager, &JDManager::databaseOutdated, this, &MainWindow::onDatabaseOutdated);

	connect(m_manager, &JDManager::startAsyncWork, this, &MainWindow::onAsyncWorkStarted);
	connect(m_manager, &JDManager::endAsyncWork, this, &MainWindow::onAsyncWorkFinished);
	connect(m_manager, &JDManager::loadObjectDone, this, &MainWindow::onLoadIndividualDone);
	connect(m_manager, &JDManager::loadObjectsDone, this, &MainWindow::onLoadAllDone);
	connect(m_manager, &JDManager::saveObjectDone, this, &MainWindow::onSaveIndividualDone);
	connect(m_manager, &JDManager::saveObjectsDone, this, &MainWindow::onSaveAllDone);



	//m_manager->getSignals().disconnect_onLoadObjectDone_slot(this, &MainWindow::onLoadIndividualDone);
	connect(&m_timer, &QTimer::timeout, this, &MainWindow::onTimerFinished);
	m_timer.start(100);

	connect(&m_asyncUpdateTimer, &QTimer::timeout, this, &MainWindow::onAsyncUpdateTimerFinished);
	//m_timer.start(1);
}

MainWindow::~MainWindow()
{
	m_timer.stop();
	m_asyncUpdateTimer.stop();
	delete m_manager;
	m_manager = nullptr;
	//JDManager::stopProfiler(m_manager->getUser()+"_asyncProfile.prof");
}

void MainWindow::onTimerFinished()
{
	EASY_FUNCTION(profiler::colors::Amber);
	//m_manager->update();
	ui.objectCount_label->setText("Object count: "+QString::number(m_manager->getObjectCount()));
	auto users = m_manager->getUsers();
	//users.push_back(Utilities::JDUser("sfefsefs", "name2", QTime(), QDate()));
	m_userListWidget->setUsers(users);

	//m_objectListWidget->setObjects(m_manager->getObjects());
	//m_objectListWidget->update();
	
}
void MainWindow::onAsyncUpdateTimerFinished()
{
	bool managerIsBusy = m_manager->isBusy();
	if (managerIsBusy)
	{
		JsonDatabase::Internal::WorkProgress progress = m_manager->getWorkProgress();
		ui.progressBar->setValue(static_cast<int>(progress.getProgress() * 100));
		ui.progressTaskName_label->setText(QString::fromStdString(progress.getTaskText()));
		ui.progressComment_label->setText(QString::fromStdString(progress.getComment()));
	}
}

void MainWindow::on_zipFormat_checkBox_stateChanged(int state)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG << "\n";
	m_manager->enableZipFormat(state);
}

void MainWindow::on_changeDatabasePath_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG << "\n";
	QString path = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
				"",
				QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);
	if (!path.isEmpty())
	{
		m_manager->setDatabasePath(path.toStdString());
	}
}

void MainWindow::on_generatePersons_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG << "\n";
	m_manager->addObject(createPersons());
}
void MainWindow::on_loadDatabase_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG << "\n";
	m_manager->loadObjectsAsync();
	//onTimerFinished();
}
void MainWindow::on_saveDatabase_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG << "\n";
	if (m_manager->isBusy())
	{
		DEBUG << "Database is busy\n"; 
		return;	
	}
	m_manager->saveLockedObjectsAsync();
	//onTimerFinished();
}
void MainWindow::on_addObject_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	JDderivedObject<Person> p(new Person("Samuel", "Richards", "Male", "30", "s.richards@randatmail.com", "666-1856-78", "Upper secondary", "Mechanic", "1", "2127", "Single", "4"));
	
	if (!m_manager->addObject(p))
	{
		DEBUG << "Can't add object to database\n";
	}
	DEBUG << p->getObjectID()->toString().c_str() << "\n";
}
void MainWindow::on_deleteObject_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	
	JDObject p = getSelectedObject();
	if (p)
	{
		DEBUG << p->getObjectID()->toString().c_str() << "\n";
		if (m_manager->removeObject(p))
		{
			JDObjectInterface* obj = p.get();
			p.reset();
		}
		//delete obj;
	}
	else
	{
		DEBUG << "Object not found\n";
	}
}
void MainWindow::on_editObject_pushButton_clicked()
{
	bool editMode = false;
	JDderivedObject<Person> p = getSelectedPerson();
	/*if (m_manager->lockObject(p))
	{
		editMode = true;
	}
	else
	{*/
	JsonDatabase::Error lastError;
	if(m_manager->lockObject(p, lastError))
	{
		editMode = true;
	}
	else
	{
		editMode = false;
	}
	if (p)
	{
		if (p->isLocked())
		{
			DEBUG << "Object is locked\n";
		}
		else
		{
			DEBUG << "Object is not locked\n";
		}
	}
	/*if (m_manager->isObjectLockedByOther(p, lastError))
	{
		editMode = false;
	}
	else
	{
		editMode = true;
	}*/
	if (lastError != JsonDatabase::Error::none)
		editMode = false;
	//}
	m_uiPersonEditor->setPerson(p, editMode);
}

void MainWindow::on_lockObject_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);

	JDObject obj = getSelectedObject();
	if (!obj)
		return;
	JsonDatabase::Error lastError;
	obj->lock();
	/*if (m_manager->lockObject(obj, lastError))
	{
		DEBUG << "locked: " << obj->getObjectID()->toString().c_str() << "\n";
	}
	else
	{
		if(obj)
			DEBUG << "Can't lock: " << obj->getObjectID()->toString().c_str() << "\n";
		else
			DEBUG << "Can't lock object, nullptr\n";
	}*/
	
	Utilities::JDUser user;
	bool isLocked = obj->getLockOwner(user);
	if (isLocked)
	{
		DEBUG << "Object is locked\n";
		DEBUG << user.toString();
	}
	else
	{
		DEBUG << "Object is not locked\n";
	}
}
void MainWindow::on_lockAllObjects_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);

	JsonDatabase::Error lastError;
	if (m_manager->lockAllObjs(lastError))
	{
		DEBUG << "locked: all objects\n";
	}
	else
	{
		DEBUG << "Can't lock all objects\n";
	}
}
void MainWindow::on_unlockObject_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);

	JDObject obj = getSelectedObject();
	JsonDatabase::Error lastError;
	if (!obj)
	{
		JDObjectID::IDType id = atoi(ui.id_lineEdit->text().toStdString().c_str());
		if (m_manager->unlockObject(id, lastError))
		{
			DEBUG << "unlocked: " << std::to_string(id) << "\n";
		}
		return;
	}
	if (m_manager->unlockObject(obj, lastError))
	{
		DEBUG << "unlocked: " << obj->getObjectID()->toString().c_str() << "\n";
	}
	else
	{
		if (obj)
			DEBUG << "Can't unlock: " << obj->getObjectID()->toString().c_str() << "\n";
		else
			DEBUG << "Can't unlock object, nullptr\n";
	}
	if (!obj)
		return;
	if (obj->isLocked())
	{
		DEBUG << "Object is locked\n";
	}
	else
	{
		DEBUG << "Object is not locked\n";
	}
}
void MainWindow::on_unlockAllObjects_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);

	JsonDatabase::Error lastError;
	if (m_manager->unlockAllObjs(lastError))
	{
		DEBUG << "unlocked: all objects\n";
	}
	else
	{
		DEBUG << "Can't unlock all objects\n";
	}
}
void MainWindow::on_test_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG << "\n";

	std::vector<JDObject> objects;
	std::unordered_map<JsonDatabase::JDObjectID::IDType, JDObject> map1;
	std::unordered_map<JDObject, JDObject> map2;

	auto added = m_manager->getObjects();

	// mesure start time using chrono 
	auto start = std::chrono::high_resolution_clock::now();
	objects.reserve(added.size());
	for (auto& obj : added)
	{
		objects.push_back(obj);
	}

	// mesure end time using chrono
	auto end = std::chrono::high_resolution_clock::now();
	// mesure duration using chrono
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	DEBUG_SIMPLE << "std::vector<JDObject> fill time: " << duration.count() << " microseconds\n";

	// mesure start time using chrono 
	start = std::chrono::high_resolution_clock::now();
	map1.reserve(added.size());
	for (auto& obj : added)
	{
		map1[obj->getObjectID()->get()] = obj;
	}

	// mesure end time using chrono
	end = std::chrono::high_resolution_clock::now();
	// mesure duration using chrono
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	DEBUG_SIMPLE << "std::unordered_map<std::string, JDObject> fill time: " << duration.count() << " microseconds\n";

	// mesure start time using chrono 
	start = std::chrono::high_resolution_clock::now();
	map2.reserve(added.size());
	for (auto& obj : added)
	{
		map2[obj] = obj;
	}

	// mesure end time using chrono
	end = std::chrono::high_resolution_clock::now();
	// mesure duration using chrono
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	DEBUG_SIMPLE << "std::unordered_map<JDObject, JDObject> fill time: " << duration.count() << " microseconds\n";


	size_t iterations = 10;
	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < iterations; ++i)
	{
		for (auto& obj : added)
		{
			auto it = std::find(objects.begin(), objects.end(), obj);
			if (it != objects.end())
			{
				//DEBUG_SIMPLE << "found\n";
			}
		}
	}
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	DEBUG_SIMPLE << "std::vector<JDObject> search time: " << duration.count() << " microseconds\n";

	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < iterations; ++i)
	{
		for (auto& obj : added)
		{
			auto it = map1.find(obj->getObjectID()->get());
			if (it != map1.end())
			{
				//DEBUG_SIMPLE << "found\n";
			}
		}
	}
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	DEBUG_SIMPLE << "std::unordered_map<std::string, JDObject> search time: " << duration.count() << " microseconds\n";

	start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < iterations; ++i)
	{
		for (auto& obj : added)
		{
			auto it = map2.find(obj);
			if (it != map2.end())
			{
				//DEBUG_SIMPLE << "found\n";
			}
		}
	}
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	DEBUG_SIMPLE << "std::unordered_map<JDObject, JDObject> search time: " << duration.count() << " microseconds\n";

}
void MainWindow::onObjectClicked(JDObject obj)
{
	ui.id_lineEdit->setText(QString::fromStdString(obj->getObjectID()->toString()));
	
	JsonDatabase::Internal::JDObjectLocker::LockData lockData;
	if (obj->getLockData(lockData))
	{
		m_lockDataWidget->setLockData(lockData);
		m_lockDataWidget->show();
	}
	else
	{
		m_lockDataWidget->hide();
	}
}
void MainWindow::closeEvent(QCloseEvent* event)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG << "\n";
	event->accept();
	emit closeWindow();
}


JDObject MainWindow::getSelectedObject()
{
#if JD_ID_TYPE_SWITCH == JD_ID_TYPE_STRING
	JDObjectID::IDType id = ui.id_lineEdit->text().toStdString();
#elif JD_ID_TYPE_SWITCH == JD_ID_TYPE_LONG
	JDObjectID::IDType id = ui.id_lineEdit->text().toLong();
#else
#error "Invalid ID type"
#endif	
	return m_manager->getObject(id);
}
JDderivedObject<Person> MainWindow::getSelectedPerson()
{
	JDObject obj = getSelectedObject();
	return std::dynamic_pointer_cast<Person>(obj);
}
void MainWindow::onDatabaseFileChanged()
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG << "\n";
	m_manager->loadObjectsAsync();
	//onTimerFinished();
}
void MainWindow::onLockedObjectsChanged()
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG << "\n";
	std::vector<JsonDatabase::Internal::JDObjectLocker::LockData> locked;
	JsonDatabase::Error lastError;
	m_manager->getObjectLocks(locked, lastError);
	std::string text;
	size_t count = 0;
	for (auto& id : locked)
	{
		auto p = m_manager->getObject<Person>(id.objectID);
		if (p.get())
		{
			text += "\"" + JDObjectID::toString(id.objectID) + "\" object locked by \"" + id.user.getName() + "\"\n";
		}
		else
			text += "\"" + JDObjectID::toString(id.objectID) + "\" object not found\n";
		++count;
		if(count > 10)
		{
			text += "...\n";
			break;
		}
	}
	ui.lockedObjects_label->setText(QString::fromStdString(text));
}
void MainWindow::onObjectRemovedFromDatabase(std::vector<JDObject>  removed)
{
	EASY_FUNCTION(profiler::colors::Amber);
	for (auto& obj : removed)
	{
		DEBUG_SIMPLE << "Removed: " << obj->getObjectID()->toString().c_str() << "\n";
	}
}
void MainWindow::onObjectAddedToDatabase(std::vector<JDObject>  added)
{
	EASY_FUNCTION(profiler::colors::Amber);
	for (auto& obj : added)
	{
		DEBUG_SIMPLE << "Added: " << obj->getObjectID()->toString().c_str() << "\n";
	}
}
/*void MainWindow::onObjectChangedFromDatabase(const std::vector<JsonDatabase::JDObjectPair>& changedPairs)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG;
	for (auto& obj : changedPairs)
	{
		DEBUG_SIMPLE << "  " << obj.first->getObjectID()->toString().c_str() << "\n";
	}
}*/
void MainWindow::onObjectChanged(std::vector<JDObject> overwritten)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG;
	for (auto& obj : overwritten)
	{
		DEBUG_SIMPLE << "  " << obj->getObjectID()->toString().c_str() << "\n";
	}
}
void MainWindow::onDatabaseOutdated()
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG;
	m_manager->loadObjectsAsync();
}


void MainWindow::onAsyncWorkStarted()
{
	m_asyncUpdateTimer.start(1);
}
void MainWindow::onAsyncWorkFinished()
{
	m_asyncUpdateTimer.stop();
	m_objectListWidget->update();
}
void MainWindow::onSaveAllDone(bool success)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG << "success: "<<(success?"true":"false") << "\n";
	if (success)
	{
		ui.progressBar->setValue(100);
		ui.progressTaskName_label->setText("Done");
		ui.progressComment_label->setText("");
	}
	else
	{
		ui.progressBar->setValue(0);
		ui.progressTaskName_label->setText("Failed");
		ui.progressComment_label->setText("");
	}
}
void MainWindow::onSaveIndividualDone(bool success, JDObject obj)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG  << obj->getObjectID()->toString().c_str() << " "<<(success ? "true" : "false") << "\n";
	if (success)
	{
		ui.progressBar->setValue(100);
		ui.progressTaskName_label->setText("Done");
		ui.progressComment_label->setText("");
	}
	else
	{
		ui.progressBar->setValue(0);
		ui.progressTaskName_label->setText("Failed");
		ui.progressComment_label->setText("");
	}
}
void MainWindow::onLoadAllDone(bool success)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG  << (success ? "true" : "false") << "\n";
	if (success)
	{
		ui.progressBar->setValue(100);
		ui.progressTaskName_label->setText("Done");
		ui.progressComment_label->setText("");
	}
	else
	{
		ui.progressBar->setValue(0);
		ui.progressTaskName_label->setText("Failed");
		ui.progressComment_label->setText("");
	}
}
void MainWindow::onLoadIndividualDone(bool success, JDObject obj)
{
	EASY_FUNCTION(profiler::colors::Amber);
	DEBUG << obj->getObjectID()->toString().c_str() <<" "<< (success ? "true" : "false") << "\n";
	if (success)
	{
		ui.progressBar->setValue(100);
		ui.progressTaskName_label->setText("Done");
		ui.progressComment_label->setText("");
	}
	else
	{
		ui.progressBar->setValue(0);
		ui.progressTaskName_label->setText("Failed");
		ui.progressComment_label->setText("");
	}
}

void MainWindow::onPersonSave(JDderivedObject<Person> person)
{
	if (person)
		m_manager->saveObjectAsync(person);
}