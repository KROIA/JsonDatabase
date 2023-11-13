#include "MainWindow.h"
#include <qdebug.h>

#ifndef JD_PROFILING
#define EASY_BLOCK(name, color)
#define EASY_END_BLOCK
#define EASY_FUNCTION(color)
#define EASY_THREAD(name)
#endif

MainWindow::MainWindow(const std::string& user, QWidget *parent)
	: QWidget(parent)
	, m_manager("asyncDatabase", "Person", "sessionID", user)
{
	ui.setupUi(this);
	//JDManager::startProfiler();

	m_manager.loadObjectsAsync();
	//m_manager.addObject(createPersons());
	//m_manager.saveObjects();

    m_manager.getSignals().connect_databaseFileChanged_slot(this, &MainWindow::onDatabaseFileChanged);
    m_manager.getSignals().connect_objectRemovedFromDatabase_slot(this, &MainWindow::onObjectRemovedFromDatabase);
    m_manager.getSignals().connect_objectAddedToDatabase_slot(this, &MainWindow::onObjectAddedToDatabase);
    m_manager.getSignals().connect_objectChangedFromDatabase_slot(this, &MainWindow::onObjectChangedFromDatabase);
    m_manager.getSignals().connect_objectOverrideChangeFromDatabase_slot(this, &MainWindow::onObjectOverrideChangeFromDatabase);
    m_manager.getSignals().connect_databaseOutdated_slot(this, &MainWindow::onDatabaseOutdated);


    m_manager.getSignals().connect_onLoadObjectsDone_slot(this, &MainWindow::onLoadAllDone);
	m_manager.getSignals().connect_onLoadObjectDone_slot(this, &MainWindow::onLoadIndividualDone);
    m_manager.getSignals().connect_onSaveObjectsDone_slot(this, &MainWindow::onSaveAllDone);
    m_manager.getSignals().connect_onSaveObjectDone_slot(this, &MainWindow::onSaveIndividualDone);



	//m_manager.getSignals().disconnect_onLoadObjectDone_slot(this, &MainWindow::onLoadIndividualDone);
	connect(&m_timer, &QTimer::timeout, this, &MainWindow::onTimerFinished);
	m_timer.start(100);
}

MainWindow::~MainWindow()
{
	//JDManager::stopProfiler(m_manager.getUser()+"_asyncProfile.prof");
}

void MainWindow::onTimerFinished()
{
	EASY_FUNCTION(profiler::colors::Amber);
	m_manager.update();
}

void MainWindow::on_loadDatabase_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	qDebug() << "on_loadDatabase_pushButton_clicked";
	m_manager.loadObjectsAsync();
	onTimerFinished();
}
void MainWindow::on_saveDatabase_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	qDebug() << "on_saveDatabase_pushButton_clicked";
	m_manager.saveObjectsAsync();
	onTimerFinished();
}
void MainWindow::on_addObject_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	
	Person* p = new Person(ui.id_lineEdit->text().toStdString(), "Samuel", "Richards", "Male", "30", "s.richards@randatmail.com", "666-1856-78", "Upper secondary", "Mechanic", "1", "2127", "Single", "4");
	qDebug() << "on_addObject_pushButton_clicked: "<<p->getObjectID().c_str();
	m_manager.addObject(p);
}
void MainWindow::on_deleteObject_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);
	
	JDObjectInterface* p = m_manager.getObject(ui.id_lineEdit->text().toStdString());
	if (p)
	{
		qDebug() << "on_deleteObject_pushButton_clicked: " << p->getObjectID().c_str();
		m_manager.removeObject(p);
		delete p;
	}
	else
	{
		qDebug() << "Object not found";
	}
}

void MainWindow::on_lockObject_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);

	JDObjectInterface *obj = m_manager.getObject(ui.id_lineEdit->text().toStdString());
	if (m_manager.lockObject(obj))
	{
		qDebug() << "locked: " << obj->getObjectID().c_str();
	}
	else
	{
		if(obj)
			qDebug() << "Can't lock: " << obj->getObjectID().c_str();
		else
			qDebug() << "Can't lock object, nullptr";
	}
}
void MainWindow::on_unlockObject_pushButton_clicked()
{
	EASY_FUNCTION(profiler::colors::Amber);

	JDObjectInterface* obj = m_manager.getObject(ui.id_lineEdit->text().toStdString());
	if (m_manager.unlockObject(obj))
	{
		qDebug() << "unlocked: " << obj->getObjectID().c_str();
	}
	else
	{
		if (obj)
			qDebug() << "Can't unlock: " << obj->getObjectID().c_str();
		else
			qDebug() << "Can't unlock object, nullptr";
	}
}





void MainWindow::onDatabaseFileChanged()
{
	EASY_FUNCTION(profiler::colors::Amber);
	qDebug() << "onDatabaseFileChanged";
	m_manager.loadObjectsAsync();
}
void MainWindow::onObjectRemovedFromDatabase(const JsonDatabase::JDObjectContainer& removed)
{
	EASY_FUNCTION(profiler::colors::Amber);
	qDebug() << "onObjectRemovedFromDatabase";
	for (auto& obj : removed)
	{
		qDebug() << "  " << obj->getObjectID().c_str();
		delete obj;
	}
}
void MainWindow::onObjectAddedToDatabase(const JsonDatabase::JDObjectContainer& added)
{
	EASY_FUNCTION(profiler::colors::Amber);
	qDebug() << "onObjectAddedToDatabase";
	for (auto& obj : added)
	{
		qDebug() << "  " << obj->getObjectID().c_str();
	}
}
void MainWindow::onObjectChangedFromDatabase(const std::vector<JsonDatabase::JDObjectPair>& changedPairs)
{
	EASY_FUNCTION(profiler::colors::Amber);
	qDebug() << "onObjectChangedFromDatabase";
	for (auto& obj : changedPairs)
	{
		qDebug() << "  " << obj.first->getObjectID().c_str();
	}
}
void MainWindow::onObjectOverrideChangeFromDatabase(const JsonDatabase::JDObjectContainer& overwritten)
{
	EASY_FUNCTION(profiler::colors::Amber);
	qDebug() << "onObjectOverrideChangeFromDatabase";
	for (auto& obj : overwritten)
	{
		qDebug() << "  " << obj->getObjectID().c_str();
	}
}
void MainWindow::onDatabaseOutdated()
{
	EASY_FUNCTION(profiler::colors::Amber);
	qDebug() << "onDatabaseOutdated";
	m_manager.loadObjectsAsync();
}



void MainWindow::onSaveAllDone(bool success)
{
	EASY_FUNCTION(profiler::colors::Amber);
	qDebug() << "onSaveAllDone success: "<<(success?"true":"false");
}
void MainWindow::onSaveIndividualDone(bool success, JDObjectInterface* obj)
{
	EASY_FUNCTION(profiler::colors::Amber);
	qDebug() << "onSaveIndividualDone: " << obj->getObjectID().c_str() << " "<<(success ? "true" : "false");;
}
void MainWindow::onLoadAllDone(bool success)
{
	EASY_FUNCTION(profiler::colors::Amber);
	qDebug() << "onLoadAllDone" << (success ? "true" : "false");;
}
void MainWindow::onLoadIndividualDone(bool success, JDObjectInterface* obj)
{
	EASY_FUNCTION(profiler::colors::Amber);
	qDebug() << "onLoadIndividualDone: " << obj->getObjectID().c_str() <<" "<< (success ? "true" : "false");;
}