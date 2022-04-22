﻿/*
    RPG Paper Maker Copyright (C) 2017-2022 Wano

    RPG Paper Maker engine is under proprietary license.
    This source code is also copyrighted.

    Use Commercial edition for commercial use of your games.
    See RPG Paper Maker EULA here:
        http://rpg-paper-maker.com/index.php/eula.
*/

#include "panelsubtransformation.h"
#include "ui_panelsubtransformation.h"
#include "rpm.h"

// -------------------------------------------------------
//
//  CONSTRUCTOR / DESTRUCTOR / GET / SET
//
// -------------------------------------------------------

PanelSubTransformation::PanelSubTransformation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PanelSubTransformation),
    m_mapElementPosition(nullptr),
    m_axisKind(AxisKind::X),
    m_drawKind(DrawKind::Translate)
{
    ui->setupUi(this);

    this->translate();
}

PanelSubTransformation::~PanelSubTransformation() {
    delete ui;
}

void PanelSubTransformation::setMapElementPosition(Position *p) {
    m_mapElementPosition = p;
}

// -------------------------------------------------------
//
//  INTERMEDIARY FUNCTIONS
//
// -------------------------------------------------------

bool PanelSubTransformation::operation() const {
    return ui->comboBoxOperation->currentIndex() == 1;
}

// -------------------------------------------------------

double PanelSubTransformation::value() const {
    return ui->doubleSpinBoxAngle->value();
}

// -------------------------------------------------------

void PanelSubTransformation::initialize(DrawKind drawKind, AxisKind ak)
{
    m_drawKind = drawKind;
    m_axisKind = ak;
    ui->comboBoxOperation->setCurrentIndex(RPM::get()->engineSettings()
        ->rotationOperation(ak) ? 1 : 0);
    ui->doubleSpinBoxAngle->setValue(RPM::get()->engineSettings()->rotationAngle
        (ak));
    ui->pushButtonDefineDefault->setText(RPM::translate(Translations
        ::DEFINE_AS_DEFAULT) + RPM::SPACE + RPM::ENUM_TO_STRING_AXIS_KIND.at(
        static_cast<int>(ak)) + RPM::SPACE + RPM::translate(Translations
        ::ROTATION).toLower());
    ui->pushButtonResetDefault->setText(RPM::translate(Translations
        ::RESET_TO_DEFAULT) + RPM::SPACE + RPM::ENUM_TO_STRING_AXIS_KIND.at(
        static_cast<int>(ak)) + RPM::SPACE + RPM::translate(Translations
        ::ROTATION).toLower());
}

// -------------------------------------------------------

void PanelSubTransformation::updatePositionAuto() {
    if (m_mapElementPosition != nullptr && !RPM::get()->engineSettings()
        ->applyLeftRightClick(m_drawKind))
    {
        Position previousPosition;

        previousPosition = *m_mapElementPosition;
        if (m_drawKind == DrawKind::Translate)
        {
            int value = qRound(this->value());
            int squares = value / RPM::getSquareSize();
            int pixels = value % RPM::getSquareSize() / 100.0;
            switch (m_axisKind)
            {
            case AxisKind::X:
                m_mapElementPosition->setX(squares);
                m_mapElementPosition->setCenterX(pixels);
                break;
            case AxisKind::Y:
                m_mapElementPosition->setY(squares);
                m_mapElementPosition->setYPlus(pixels);
                break;
            case AxisKind::Z:
                m_mapElementPosition->setZ(squares);
                m_mapElementPosition->setCenterZ(pixels);
                break;
            }
        } else if (m_drawKind == DrawKind::Rotate)
        {
            m_mapElementPosition->setAngle(m_axisKind, this->value());
        }
        if (*m_mapElementPosition != previousPosition)
        {
            emit positionChanged(previousPosition);
        }
    }
}

// -------------------------------------------------------

void PanelSubTransformation::updatePositionClick(bool positive)
{
    if (m_mapElementPosition != nullptr && RPM::get()->engineSettings()
            ->applyLeftRightClick(m_drawKind))
    {
        Position previousPosition;

        previousPosition = *m_mapElementPosition;
        if (m_drawKind == DrawKind::Translate)
        {
            int value = qRound(this->value());
            int squares = value / RPM::getSquareSize();
            int pixels = value % RPM::getSquareSize() / 100.0;
            squares = positive ? squares : -squares;
            pixels = positive ? pixels : -pixels;
            switch (m_axisKind)
            {
            case AxisKind::X:
                if (this->operation())
                {
                    m_mapElementPosition->setX(squares);
                    m_mapElementPosition->setCenterX(pixels);
                } else
                {
                    m_mapElementPosition->addX(squares);
                    m_mapElementPosition->addCenterX(pixels);
                }
                m_mapElementPosition->setX(qMax(qMin(0, m_mapElementPosition->x()),
                    RPM::get()->project()->currentMap()->mapProperties()->length()));
                break;
            case AxisKind::Y:
                if (this->operation())
                {
                    m_mapElementPosition->setY(squares);
                    m_mapElementPosition->setYPlus(pixels);
                } else
                {
                    m_mapElementPosition->addY(squares);
                    m_mapElementPosition->addYPlus(pixels);
                }
                m_mapElementPosition->setY(qMax(qMin(-RPM::get()->project()
                    ->currentMap()->mapProperties()->depth(), m_mapElementPosition->y()),
                    RPM::get()->project()->currentMap()->mapProperties()->height()));
                break;
            case AxisKind::Z:
                if (this->operation())
                {
                    m_mapElementPosition->setZ(squares);
                    m_mapElementPosition->setCenterZ(pixels);
                } else
                {
                    m_mapElementPosition->addZ(squares);
                    m_mapElementPosition->addCenterZ(pixels);
                }
                m_mapElementPosition->setZ(qMax(qMin(0, m_mapElementPosition->z()),
                    RPM::get()->project()->currentMap()->mapProperties()->width()));
                break;
            }
        } else if (m_drawKind == DrawKind::Rotate)
        {
            if (this->operation()) {
                m_mapElementPosition->setAngle(m_axisKind, positive ? this->value()
                    : -this->value());
            } else {
                m_mapElementPosition->addAngle(m_axisKind, positive ? this->value()
                    : -this->value());
            }
        }

        if (*m_mapElementPosition != previousPosition) {
            emit positionChanged(previousPosition);
        }
    }
}

//-------------------------------------------------

void PanelSubTransformation::updateApplyLeftRightEnabled(bool checked)
{
    QModelIndex modelIndex = ui->comboBoxOperation->model()->index(0, 0);
    if (!checked)
    {
        QVariant vDisable(0);
        ui->comboBoxOperation->setCurrentIndex(1);
        ui->comboBoxOperation->model()->setData(modelIndex, vDisable, Qt
            ::UserRole - 1);
    } else
    {
        QVariant vEnable(1 | 32);
        ui->comboBoxOperation->model()->setData(modelIndex, vEnable, Qt
            ::UserRole - 1);
    }
}

//-------------------------------------------------

void PanelSubTransformation::translate()
{
    ui->labelAngle->setText(RPM::translate(Translations::ANGLE) + RPM::COLON);
}

// -------------------------------------------------------
//
//  SLOTS
//
// -------------------------------------------------------

void PanelSubTransformation::on_comboBoxOperation_currentIndexChanged(int index)
{
    bool equal;

    equal = index == 1;
    RPM::get()->engineSettings()->setRotationOperation(m_axisKind, equal);
    RPM::get()->engineSettings()->write();
    if (equal) {
        this->updatePositionAuto();
    }
}

// -------------------------------------------------------

void PanelSubTransformation::on_doubleSpinBoxAngle_valueChanged(double) {
    this->updatePositionAuto();
}

// -------------------------------------------------------

void PanelSubTransformation::on_pushButtonDefineDefault_clicked() {
    RPM::get()->engineSettings()->setRotationAngle(m_axisKind, this->value());
    RPM::get()->engineSettings()->write();
}

// -------------------------------------------------------

void PanelSubTransformation::on_pushButtonResetDefault_clicked() {
    ui->doubleSpinBoxAngle->setValue(RPM::get()->engineSettings()->rotationAngle
        (m_axisKind));
    this->updatePositionAuto();
}
