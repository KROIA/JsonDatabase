#pragma once

#include <QWidget>
#include <qtimer.h>
#include "ui_MainWindow.h"
#include "JsonDatabase.h"
#include "Person.h"

class MainWindow : public QWidget
{
	Q_OBJECT

public:
	MainWindow(const std::string &user, QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void onTimerFinished();

	void on_loadDatabase_pushButton_clicked();
	void on_saveDatabase_pushButton_clicked();
	void on_addObject_pushButton_clicked();
	void on_deleteObject_pushButton_clicked();

	void on_lockObject_pushButton_clicked();
	void on_unlockObject_pushButton_clicked();
private:
	// Signals from the manager
	void onDatabaseFileChanged();
	void onObjectRemovedFromDatabase(const JsonDatabase::JDObjectContainer& removed);
	void onObjectAddedToDatabase(const JsonDatabase::JDObjectContainer& added);
	void onObjectChangedFromDatabase(const std::vector<JsonDatabase::JDObjectPair>&changedPairs);
	void onObjectOverrideChangeFromDatabase(const JsonDatabase::JDObjectContainer& overwritten);
	void onDatabaseOutdated();

	void onSaveAllDone(bool success);
	void onSaveIndividualDone(bool success, JDObjectInterface*obj);
	void onLoadAllDone(bool success);
	void onLoadIndividualDone(bool success, JDObjectInterface*obj);



	Ui::MainWindow ui;
	QTimer m_timer;

	JDManager m_manager;
};
