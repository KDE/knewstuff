install(FILES
   thumb_frame.png
   DESTINATION ${KDE_INSTALL_DATADIR_KF5}/knewstuff/pics)

# Exclude tool to aggregate updated desktop files from being installed. It
# serves no purpose in production.
install(DIRECTORY
    kmoretools-desktopfiles/
    DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kmoretools/presets-kmoretools
    PATTERN "*.sh" EXCLUDE)
