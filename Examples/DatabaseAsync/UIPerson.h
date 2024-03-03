#pragma once

#include <QWidget>
#include "ui_UIPerson.h"
#include "Person.h"

class UIPerson : public QWidget
{
	Q_OBJECT

public:
	UIPerson(QWidget *parent = nullptr);
	~UIPerson();


	void setPerson(JDderivedObject<Person> person, bool editMode);
signals:
	void savePerson(JDderivedObject<Person> person);
private slots:

	void on_save_pushButton_clicked();
	
private:
	Ui::UIPerson ui;
	JDderivedObject<Person> m_person;
	bool m_editMode;

};
