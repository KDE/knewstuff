# install .desktop files executable, otherwise they are not usable if not root owned
install(DIRECTORY
    kmoretools-desktopfiles/
    DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kmoretools/presets-kmoretools
    FILES_MATCHING PATTERN "*.desktop"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
    )

# install other files like icons, skip .sh helpers
install(DIRECTORY
    kmoretools-desktopfiles/
    DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kmoretools/presets-kmoretools
    PATTERN "*.desktop" EXCLUDE
    PATTERN "*.sh" EXCLUDE)
