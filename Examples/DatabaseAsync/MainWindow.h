#pragma once

#include <QWidget>
#include <qtimer.h>
#include "ui_MainWindow.h"
#include "JsonDatabase.h"
#include "Person.h"
#include "UIPerson.h"

class MainWindow : public QWidget
{
	Q_OBJECT

public:
	MainWindow(const std::string &user, QWidget *parent = nullptr);
	~MainWindow();

signals:
	void closeWindow();
private slots:
	void onTimerFinished();
	void onAsyncUpdateTimerFinished();

	void on_zipFormat_checkBox_stateChanged(int state);

	void on_generatePersons_pushButton_clicked();
	void on_loadDatabase_pushButton_clicked();
	void on_saveDatabase_pushButton_clicked();
	void on_addObject_pushButton_clicked();
	void on_deleteObject_pushButton_clicked();
	void on_editObject_pushButton_clicked();

	void on_lockObject_pushButton_clicked();
	void on_unlockObject_pushButton_clicked();
	void on_test_pushButton_clicked();

	void onPersonSave(JDderivedObject<Person> person);
private:
	// catch the close event
	void closeEvent(QCloseEvent* event) override;

	JDObject getSelectedObject();
	JDderivedObject<Person>getSelectedPerson();

	// Signals from the manager
	void onDatabaseFileChanged();
	void onLockedObjectsChanged();
	void onObjectRemovedFromDatabase(const std::vector<JDObject>& removed);
	void onObjectAddedToDatabase(const std::vector<JDObject>& added);
	void onObjectChangedFromDatabase(const std::vector<JsonDatabase::JDObjectPair>&changedPairs);
	void onObjectOverrideChangeFromDatabase(const std::vector<JDObject>& overwritten);
	void onDatabaseOutdated();

	void onAsyncWorkStarted();
	void onAsyncWorkFinished();
	void onSaveAllDone(bool success);
	void onSaveIndividualDone(bool success, JDObject obj);
	void onLoadAllDone(bool success);
	void onLoadIndividualDone(bool success, JDObject obj);



	Ui::MainWindow ui;
	QTimer m_timer;
	QTimer m_asyncUpdateTimer;

	JDManager *m_manager;
	UIPerson* m_uiPersonEditor;
};
