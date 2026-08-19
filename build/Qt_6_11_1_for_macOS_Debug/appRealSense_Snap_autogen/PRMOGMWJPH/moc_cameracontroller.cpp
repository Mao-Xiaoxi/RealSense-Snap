/****************************************************************************
** Meta object code from reading C++ file 'cameracontroller.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/cameracontroller.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cameracontroller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN16CameraControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto CameraController::qt_create_metaobjectdata<qt_meta_tag_ZN16CameraControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "CameraController",
        "alphaChanged",
        "",
        "alphaRequested",
        "a",
        "camerasChanged",
        "cameraStatusChanged",
        "selectedCameraSerialChanged",
        "refreshDevicesRequested",
        "cameraSelected",
        "serial",
        "backgroundImageRequested",
        "path",
        "setAlpha",
        "refreshDevices",
        "setSelectedCameraSerial",
        "setDevices",
        "QVariantList",
        "devices",
        "setCameraStatus",
        "message",
        "setSelectedCameraSerialFromWorker",
        "setBackgroundImage",
        "alpha",
        "cameras",
        "cameraStatus",
        "selectedCameraSerial"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'alphaChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'alphaRequested'
        QtMocHelpers::SignalData<void(float)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 4 },
        }}),
        // Signal 'camerasChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'cameraStatusChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'selectedCameraSerialChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'refreshDevicesRequested'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'cameraSelected'
        QtMocHelpers::SignalData<void(QString)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Signal 'backgroundImageRequested'
        QtMocHelpers::SignalData<void(QString)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 12 },
        }}),
        // Slot 'setAlpha'
        QtMocHelpers::SlotData<void(float)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 4 },
        }}),
        // Slot 'refreshDevices'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setSelectedCameraSerial'
        QtMocHelpers::SlotData<void(QString)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Slot 'setDevices'
        QtMocHelpers::SlotData<void(QVariantList)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 17, 18 },
        }}),
        // Slot 'setCameraStatus'
        QtMocHelpers::SlotData<void(QString)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 20 },
        }}),
        // Slot 'setSelectedCameraSerialFromWorker'
        QtMocHelpers::SlotData<void(QString)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Slot 'setBackgroundImage'
        QtMocHelpers::SlotData<void(QString)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 12 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'alpha'
        QtMocHelpers::PropertyData<float>(23, QMetaType::Float, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'cameras'
        QtMocHelpers::PropertyData<QVariantList>(24, 0x80000000 | 17, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 2),
        // property 'cameraStatus'
        QtMocHelpers::PropertyData<QString>(25, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'selectedCameraSerial'
        QtMocHelpers::PropertyData<QString>(26, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CameraController, qt_meta_tag_ZN16CameraControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject CameraController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16CameraControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16CameraControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16CameraControllerE_t>.metaTypes,
    nullptr
} };

void CameraController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CameraController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->alphaChanged(); break;
        case 1: _t->alphaRequested((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 2: _t->camerasChanged(); break;
        case 3: _t->cameraStatusChanged(); break;
        case 4: _t->selectedCameraSerialChanged(); break;
        case 5: _t->refreshDevicesRequested(); break;
        case 6: _t->cameraSelected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->backgroundImageRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->setAlpha((*reinterpret_cast<std::add_pointer_t<float>>(_a[1]))); break;
        case 9: _t->refreshDevices(); break;
        case 10: _t->setSelectedCameraSerial((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->setDevices((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1]))); break;
        case 12: _t->setCameraStatus((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->setSelectedCameraSerialFromWorker((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->setBackgroundImage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CameraController::*)()>(_a, &CameraController::alphaChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraController::*)(float )>(_a, &CameraController::alphaRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraController::*)()>(_a, &CameraController::camerasChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraController::*)()>(_a, &CameraController::cameraStatusChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraController::*)()>(_a, &CameraController::selectedCameraSerialChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraController::*)()>(_a, &CameraController::refreshDevicesRequested, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraController::*)(QString )>(_a, &CameraController::cameraSelected, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (CameraController::*)(QString )>(_a, &CameraController::backgroundImageRequested, 7))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<float*>(_v) = _t->alpha(); break;
        case 1: *reinterpret_cast<QVariantList*>(_v) = _t->cameras(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->cameraStatus(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->selectedCameraSerial(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setAlpha(*reinterpret_cast<float*>(_v)); break;
        case 3: _t->setSelectedCameraSerial(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *CameraController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CameraController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16CameraControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CameraController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 15;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void CameraController::alphaChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CameraController::alphaRequested(float _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void CameraController::camerasChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void CameraController::cameraStatusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void CameraController::selectedCameraSerialChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void CameraController::refreshDevicesRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void CameraController::cameraSelected(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void CameraController::backgroundImageRequested(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}
QT_WARNING_POP
