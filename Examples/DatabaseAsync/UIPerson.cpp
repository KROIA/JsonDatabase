#include "UIPerson.h"
#include <qdebug.h>

#ifndef JD_PROFILING
#define EASY_BLOCK(name, color)
#define EASY_END_BLOCK
#define EASY_FUNCTION(color)
#define EASY_THREAD(name)
#endif

UIPerson::UIPerson(QWidget *parent)
	: QWidget(parent)
	, m_person(nullptr)
{
	ui.setupUi(this);
	
}

UIPerson::~UIPerson()
{

}

void UIPerson::setPerson(Person* person, bool editMode)
{
	m_editMode = editMode;
	if (!m_editMode)
	{
		ui.frame->setEnabled(false);
	}
	else
	{
		ui.frame->setEnabled(true);
	}
	m_person = person;
	if (m_person)
	{
		ui.objectID_label->setText(m_person->getObjectID().toQString());
		ui.firstName_lineEdit->setText(QString::fromStdString(m_person->firstName));
		ui.lastName_lineEdit->setText(QString::fromStdString(m_person->lastName));
	}
	else
	{
		ui.objectID_label->setText("");
		ui.firstName_lineEdit->setText("");
		ui.lastName_lineEdit->setText("");
	}
}

void UIPerson::on_save_pushButton_clicked()
{
	if (!m_person || !m_editMode)
		return;

	m_person->firstName = ui.firstName_lineEdit->text().toStdString();
	m_person->lastName = ui.lastName_lineEdit->text().toStdString();
}
