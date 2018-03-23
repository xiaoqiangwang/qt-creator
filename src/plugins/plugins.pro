include(../../qtcreator.pri)

TEMPLATE  = subdirs

SUBDIRS   = \
    coreplugin \
    texteditor \
    diffeditor \
    projectexplorer \
    vcsbase \
    git \
    qtsupport \
    cpptools \
    qmakeprojectmanager \
    resourceeditor \
    qmljseditor \
    qmljstools

QTC_DO_NOT_BUILD_QMLDESIGNER = $$(QTC_DO_NOT_BUILD_QMLDESIGNER)
isEmpty(QTC_DO_NOT_BUILD_QMLDESIGNER):qtHaveModule(quick-private) {
    exists($$[QT_INSTALL_QML]/QtQuick/Controls/qmldir) {
       SUBDIRS += qmldesigner
    } else {
        warning("QmlDesigner plugin has been disabled since Qt Quick Controls 1 are not installed.")
    }
} else {
    !qtHaveModule(quick-private) {
        warning("QmlDesigner plugin has been disabled since the Qt Quick module is not available.")
    } else {
        warning("QmlDesigner plugin has been disabled since QTC_DO_NOT_BUILD_QMLDESIGNER is set.")
    }
}

for(p, SUBDIRS) {
    QTC_PLUGIN_DEPENDS =
    include($$p/$${p}_dependencies.pri)
    pv = $${p}.depends
    $$pv = $$QTC_PLUGIN_DEPENDS
}
