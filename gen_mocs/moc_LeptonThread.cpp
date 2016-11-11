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
      36,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   33,   33,   33, 0x05,
      34,   33,   33,   33, 0x05,
      62,   33,   33,   33, 0x05,
      94,   33,   33,   33, 0x05,
     118,   33,   33,   33, 0x05,

 // slots: signature, parameters, type, tag, flags
     138,   33,   33,   33, 0x0a,
     151,   33,   33,   33, 0x0a,
     163,   33,   33,   33, 0x0a,
     176,   33,   33,   33, 0x0a,
     202,   33,   33,   33, 0x0a,
     225,   33,   33,   33, 0x0a,
     248,   33,   33,   33, 0x0a,
     266,   33,   33,   33, 0x0a,
     284,  316,   33,   33, 0x0a,
     322,  316,   33,   33, 0x0a,
     353,   33,   33,   33, 0x0a,
     375,   33,   33,   33, 0x0a,
     394,   33,   33,   33, 0x0a,
     410,   33,   33,   33, 0x0a,
     427,   33,   33,   33, 0x0a,
     444,   33,   33,   33, 0x0a,
     464,   33,   33,   33, 0x0a,
     483,   33,   33,   33, 0x0a,
     504,   33,   33,   33, 0x0a,
     520,   33,   33,   33, 0x0a,
     548,   33,   33,   33, 0x0a,
     564,   33,   33,   33, 0x0a,
     585,   33,   33,   33, 0x0a,
     604,   33,   33,   33, 0x0a,
     613,   33,   33,   33, 0x0a,
     622,  681,  701,   33, 0x0a,
     708,   33,   33,   33, 0x0a,
     724,  751,   33,   33, 0x0a,
     757,  783,  701,   33, 0x0a,
     794,   33,   33,   33, 0x0a,
     810,  828,  839,   33, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_LeptonThread[] = {
    "LeptonThread\0updateText(QString)\0\0"
    "updateTextContours(QString)\0"
    "updateTextContoursHull(QString)\0"
    "updateTextReco(QString)\0updateImage(QImage)\0"
    "performFFC()\0enableAGC()\0disableAGC()\0"
    "change_colormap_rainbow()\0"
    "change_colormap_gray()\0change_colormap_iron()\0"
    "set_normal_mode()\0set_binary_mode()\0"
    "change_slider_value_binary(int)\0value\0"
    "change_slider_value_canny(int)\0"
    "switchon_dilatation()\0switchon_erosion()\0"
    "switchon_open()\0switchon_close()\0"
    "switchon_sobel()\0switchon_skeleton()\0"
    "switchon_mediane()\0switchon_histogram()\0"
    "switchon_hull()\0switchon_conting_countour()\0"
    "switchon_line()\0switchon_recognize()\0"
    "switchon_rescale()\0get_BG()\0sub_BG()\0"
    "draw_convex_hull(Mat,std::vector<std::vector<Point> >,int)\0"
    "image,conto,biggest\0double\0make_snapshot()\0"
    "histogram_alternative(Mat)\0image\0"
    "counting_contour(Mat,Mat)\0image,mask\0"
    "separate_hand()\0rescale_hand(Mat)\0"
    "image_mask\0Mat\0"
};

void LeptonThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        LeptonThread *_t = static_cast<LeptonThread *>(_o);
        switch (_id) {
        case 0: _t->updateText((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->updateTextContours((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->updateTextContoursHull((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->updateTextReco((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->updateImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 5: _t->performFFC(); break;
        case 6: _t->enableAGC(); break;
        case 7: _t->disableAGC(); break;
        case 8: _t->change_colormap_rainbow(); break;
        case 9: _t->change_colormap_gray(); break;
        case 10: _t->change_colormap_iron(); break;
        case 11: _t->set_normal_mode(); break;
        case 12: _t->set_binary_mode(); break;
        case 13: _t->change_slider_value_binary((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->change_slider_value_canny((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->switchon_dilatation(); break;
        case 16: _t->switchon_erosion(); break;
        case 17: _t->switchon_open(); break;
        case 18: _t->switchon_close(); break;
        case 19: _t->switchon_sobel(); break;
        case 20: _t->switchon_skeleton(); break;
        case 21: _t->switchon_mediane(); break;
        case 22: _t->switchon_histogram(); break;
        case 23: _t->switchon_hull(); break;
        case 24: _t->switchon_conting_countour(); break;
        case 25: _t->switchon_line(); break;
        case 26: _t->switchon_recognize(); break;
        case 27: _t->switchon_rescale(); break;
        case 28: _t->get_BG(); break;
        case 29: _t->sub_BG(); break;
        case 30: { double _r = _t->draw_convex_hull((*reinterpret_cast< Mat(*)>(_a[1])),(*reinterpret_cast< std::vector<std::vector<Point> >(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< double*>(_a[0]) = _r; }  break;
        case 31: _t->make_snapshot(); break;
        case 32: _t->histogram_alternative((*reinterpret_cast< Mat(*)>(_a[1]))); break;
        case 33: { double _r = _t->counting_contour((*reinterpret_cast< Mat(*)>(_a[1])),(*reinterpret_cast< Mat(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< double*>(_a[0]) = _r; }  break;
        case 34: _t->separate_hand(); break;
        case 35: { Mat _r = _t->rescale_hand((*reinterpret_cast< Mat(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< Mat*>(_a[0]) = _r; }  break;
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
        if (_id < 36)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 36;
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
void LeptonThread::updateTextContours(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void LeptonThread::updateTextContoursHull(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void LeptonThread::updateTextReco(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void LeptonThread::updateImage(QImage _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_END_MOC_NAMESPACE
