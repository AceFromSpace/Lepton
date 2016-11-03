/****************************************************************************
** Meta object code from reading C++ file 'LeptonThread.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../LeptonThread.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LeptonThread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_LeptonThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   33,   33,   33, 0x05,
      34,   33,   33,   33, 0x05,

 // slots: signature, parameters, type, tag, flags
      54,   33,   33,   33, 0x0a,
      67,   33,   33,   33, 0x0a,
      79,   33,   33,   33, 0x0a,
      92,   33,   33,   33, 0x0a,
     118,   33,   33,   33, 0x0a,
     141,   33,   33,   33, 0x0a,
     164,   33,   33,   33, 0x0a,
     182,   33,   33,   33, 0x0a,
     200,  225,   33,   33, 0x0a,
     231,   33,   33,   33, 0x0a,
     253,   33,   33,   33, 0x0a,
     272,   33,   33,   33, 0x0a,
     288,   33,   33,   33, 0x0a,
     305,   33,   33,   33, 0x0a,
     322,   33,   33,   33, 0x0a,
     342,   33,   33,   33, 0x0a,
     359,   33,   33,   33, 0x0a,
     377,   33,   33,   33, 0x0a,
     396,   33,   33,   33, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_LeptonThread[] = {
    "LeptonThread\0updateText(QString)\0\0"
    "updateImage(QImage)\0performFFC()\0"
    "enableAGC()\0disableAGC()\0"
    "change_colormap_rainbow()\0"
    "change_colormap_gray()\0change_colormap_iron()\0"
    "set_normal_mode()\0set_binary_mode()\0"
    "change_slider_value(int)\0value\0"
    "switchon_dilatation()\0switchon_erosion()\0"
    "switchon_open()\0switchon_close()\0"
    "switchon_sobel()\0switchon_skeleton()\0"
    "switchon_learn()\0switchoff_learn()\0"
    "switchon_mediane()\0make_snapshot()\0"
};

void LeptonThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        LeptonThread *_t = static_cast<LeptonThread *>(_o);
        switch (_id) {
        case 0: _t->updateText((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->updateImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 2: _t->performFFC(); break;
        case 3: _t->enableAGC(); break;
        case 4: _t->disableAGC(); break;
        case 5: _t->change_colormap_rainbow(); break;
        case 6: _t->change_colormap_gray(); break;
        case 7: _t->change_colormap_iron(); break;
        case 8: _t->set_normal_mode(); break;
        case 9: _t->set_binary_mode(); break;
        case 10: _t->change_slider_value((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->switchon_dilatation(); break;
        case 12: _t->switchon_erosion(); break;
        case 13: _t->switchon_open(); break;
        case 14: _t->switchon_close(); break;
        case 15: _t->switchon_sobel(); break;
        case 16: _t->switchon_skeleton(); break;
        case 17: _t->switchon_learn(); break;
        case 18: _t->switchoff_learn(); break;
        case 19: _t->switchon_mediane(); break;
        case 20: _t->make_snapshot(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData LeptonThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject LeptonThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_LeptonThread,
      qt_meta_data_LeptonThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &LeptonThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *LeptonThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *LeptonThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LeptonThread))
        return static_cast<void*>(const_cast< LeptonThread*>(this));
    return QThread::qt_metacast(_clname);
}

int LeptonThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    }
    return _id;
}

// SIGNAL 0
void LeptonThread::updateText(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LeptonThread::updateImage(QImage _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
