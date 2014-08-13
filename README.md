# KNewStuff

Framework for downloading and sharing additional application data

## Introduction

The KNewStuff library implements collaborative data sharing for
applications. It uses libattica to support the Open Collaboration Services
specification.


## Usage

If you are using CMake, you need to have

    find_package(KF5NewStuff NO_MODULE)

(or similar) in your CMakeLists.txt file, and you need to link to KF5::NewStuff.

Application authors should use KNS3::DownloadDialog for downloading application
content.  For uploading KNS3::UploadDialog is used.

Related information such as creation of <b>*.knsrc</b> files can be found on
techbase in the [Get Hot New Stuff
tutorials](http://techbase.kde.org/Development/Tutorials#Get_Hot_New_Stuff).



