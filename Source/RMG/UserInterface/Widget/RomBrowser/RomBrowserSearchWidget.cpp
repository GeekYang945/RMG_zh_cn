/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020-2026 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "RomBrowserSearchWidget.hpp"

#include <QEvent>

using namespace UserInterface::Widget;

RomBrowserSearchWidget::RomBrowserSearchWidget(QWidget* parent) : QWidget(parent)
{
	// configure search line edit
	this->lineEdit = new QLineEdit(this);
    connect(this->lineEdit, &QLineEdit::textChanged, this, &RomBrowserSearchWidget::on_lineEdit_textChanged);

    // configure close button
	this->closeButton = new QPushButton(this);
    connect(this->closeButton, &QPushButton::clicked, this, &RomBrowserSearchWidget::on_closeButton_clicked);

    // configure layout
	this->widgetLayout = new QHBoxLayout(this);
    this->widgetLayout->addWidget(this->lineEdit);
    this->widgetLayout->addWidget(this->closeButton);
    this->setLayout(this->widgetLayout);

    this->RetranslateUI();
}

RomBrowserSearchWidget::~RomBrowserSearchWidget(void)
{
}

void RomBrowserSearchWidget::FocusSearchText(void)
{
	this->lineEdit->setFocus();
}

void RomBrowserSearchWidget::RetranslateUI(void)
{
    this->lineEdit->setPlaceholderText(tr("Search games..."));
    this->closeButton->setText(tr("Close"));
}

void RomBrowserSearchWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        this->RetranslateUI();
    }

    QWidget::changeEvent(event);
}

void RomBrowserSearchWidget::ClearSearchTerm(void)
{
	this->lineEdit->setText("");
}

void RomBrowserSearchWidget::on_lineEdit_textChanged(const QString& text)
{
	emit this->SearchTextChanged(text);
}

void RomBrowserSearchWidget::on_closeButton_clicked(void)
{
	this->hide();
	this->ClearSearchTerm();
}

