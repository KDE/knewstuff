# KNewStuff

Framework for downloading and sharing additional application data

## Introduction

The KNewStuff library implements collaborative data sharing for
applications. It uses libattica to support the Open Collaboration Services
specification.


## Usage

There are three parts to KNewStuff:

* *KNewStuffCore* - The core functionality, which takes care of the actual work
  (downloading data and interacting with the remote services). Importantly, this
  library has no dependencies past Tier 1, and so while the entire framework is
  to be considered Tier 3, the KNewStuffCore library can be considered Tier 2
  for integration purposes.
* *KNewStuff* - A Qt Widget based UI library, designed for ease of implementation of
  various UI patterns found through KDE applications (such as the Get New Stuff buttons,
  as well as generic download and upload dialogues)
* *KNewStuffQuick* - A set of Qt Quick based components designed to provide similar
  pattern support as KNewStuff, except for Qt Quick based applications, and specifically
  in Kirigami based applications.

If you are using CMake, you need to find the modules, which can be done by doing one of
the following in your CMakeLists.txt:

Either use the more modern (and compact) component based method (only actually add the
component you need, since both NewStuff and NewStuffQuick depend on NewStuffCore):

    find_package(KF5 COMPONENTS NewStuffCore NewStuff NewStuffQuick)

Or use the old-fashioned syntax

    find_package(KF5NewStuffCore CONFIG) # for the KNewStuffCore library only
    find_package(KF5NewStuff CONFIG) # for the KNewStuff UI library, will pull in KNewStuffCore for you
    find_package(KF5NewStuffQuick CONFIG) # for the KNewStuffQuick UI library, will pull in KNewStuffCore for you

Also remember to link to the library you are using (either KF5::NewStuff or
KF5::NewStuffCore), and for the Qt Quick NewStuffQuick module, add the following
to the QML files where you wish to use the components:

    import org.kde.newstuff 1.0

Finally, because KNewStuffQuick is not a link time requirement, it would be good form
to mark it as a runtime requirement (and describing why you need them), which is done
by adding the following in your CMakeLists.txt sometime after the find statement:

    set_package_properties(KF5NewStuffQuick PROPERTIES
        DESCRIPTION "Qt Quick components used for interacting with remote data services"
        URL "https://api.kde.org/frameworks/knewstuff/html/index.html"
        PURPOSE "Required to Get Hot New Stuff for my applicaton"
        TYPE RUNTIME)

When installing your knsrc configuration file, you should install it into the location
where KNewStuffCore expects it to be found. Do this by using the CMake variable
KDE_INSTALL_KNSRCDIR as provided by the KNewStuffCore module. You can also handle this
yourself, which means you will need to feed Engine::init() the full path to the knsrc file.

## Which module should you use?

When building applications designed to fit in with other classic, widget based
applications, the application authors should use KNS3::DownloadDialog for
downloading application content. For uploading KNS3::UploadDialog is used.

When building Qt Quick (and in particular Kirigami) based applications, you can
use the NewStuffList item from the org.kde.newstuff import to achieve a similar
functionality to KNS3::DownloadDialog. You can also use the ItemsModel directly,
if this is not featureful enough. Uploading is currently not exposed in KNewStuffQuick.

If neither of these options are powerful enough for your needs, you can access
the functionality directly through the classes in the KNSCore namespace.

Related information such as creation of <b>*.knsrc</b> files can be found on
techbase in the [Get Hot New Stuff
tutorials](https://techbase.kde.org/Development/Tutorials#Get_Hot_New_Stuff).
