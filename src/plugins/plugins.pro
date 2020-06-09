include(../../qtcreator.pri)

TEMPLATE  = subdirs

SUBDIRS   = \
    coreplugin \
    texteditor \
    diffeditor \
    projectexplorer \
    vcsbase \
    git \
    cpptools \
    qtsupport \
    qmakeprojectmanager \
    resourceeditor \
    qmljseditor \
    qmljstools \
    qmlpreview

DO_NOT_BUILD_QMLDESIGNER = $$(DO_NOT_BUILD_QMLDESIGNER)
isEmpty(DO_NOT_BUILD_QMLDESIGNER):qtHaveModule(quick-private) {
    exists($$[QT_INSTALL_QML]/QtQuick/Controls/qmldir) {
       SUBDIRS += qmldesigner
    } else {
        warning("QmlDesigner plugin has been disabled since Qt Quick Controls 1 are not installed.")
    }
} else {
    !qtHaveModule(quick-private) {
        warning("QmlDesigner plugin has been disabled since the Qt Quick module is not available.")
    } else {
        warning("QmlDesigner plugin has been disabled since DO_NOT_BUILD_QMLDESIGNER is set.")
    }
}


for(p, SUBDIRS) {
    QTC_PLUGIN_DEPENDS =
    include($$p/$${p}_dependencies.pri)
    pv = $${p}.depends
    $$pv = $$QTC_PLUGIN_DEPENDS
}

QMAKE_EXTRA_TARGETS += deployqt # dummy
