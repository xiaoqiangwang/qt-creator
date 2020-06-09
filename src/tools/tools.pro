include(../../qtcreator.pri)

TEMPLATE = subdirs

qtHaveModule(quick-private): SUBDIRS += qml2puppet
