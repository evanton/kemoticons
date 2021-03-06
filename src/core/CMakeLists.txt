############# kemoticons library ###################

set(kemoticons_LIB_SRCS
    kemoticons.cpp
    kemoticonstheme.cpp
    kemoticonsprovider.cpp
    kemoticonsglobal.cpp
)

add_library(KF5Emoticons ${kemoticons_LIB_SRCS})
generate_export_header(KF5Emoticons BASE_NAME KEmoticons)
add_library(KF5::Emoticons ALIAS KF5Emoticons)

target_include_directories(KF5Emoticons INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KEmoticons>")

target_link_libraries(KF5Emoticons
PUBLIC
    Qt5::Gui # for QImageReader, QPixmap
    KF5::Service     # for KService::Ptr, KServiceTypeTrader
PRIVATE
    KF5::Archive    # for KTar, KZip
    Qt5::DBus
)

set_target_properties(KF5Emoticons PROPERTIES
                      VERSION ${KEMOTICONS_VERSION_STRING}
                      SOVERSION ${KEMOTICONS_SOVERSION}
                      EXPORT_NAME Emoticons
)

install(FILES kemoticonsTheme.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})

ecm_generate_headers(KEmoticons_HEADERS
  HEADER_NAMES
  KEmoticons
  KEmoticonsTheme
  KEmoticonsProvider

  REQUIRED_HEADERS KEmoticons_HEADERS
)


install(TARGETS KF5Emoticons EXPORT KF5EmoticonsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kemoticons_export.h
  ${KEmoticons_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KEmoticons
  COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF5Emoticons_QCH
        NAME KEmoticons
        BASE_NAME KF5Emoticons
        VERSION ${KF5_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KEmoticons_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Gui_QCH
            KF5Service_QCH
        BLANK_MACROS
            KEMOTICONS_EXPORT
            KEMOTICONS_DEPRECATED
            KEMOTICONS_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KEmoticons LIB_NAME KF5Emoticons DEPS "widgets KService" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KEmoticons)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
