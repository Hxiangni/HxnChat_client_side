cmake_minimum_required(VERSION 3.19)
project(HxnChat LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
find_package(spdlog REQUIRED)

# ============================================================
# 开启 AUTOMOC、AUTOUIC、AUTORCC；
# 自动处理UI、QRC等
# ============================================================
qt_standard_project_setup()


# ============================================================
# 设置项目一些默认编译属性
# Qt6 封装的创建可执行程序命令，替代原生add_executable。
# 把rc资源文件加入编译
# ============================================================
qt_add_executable(HxnChat
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    logindialog.h
    logindialog.cpp
    logindialog.ui
    app.rc
    global.h global.cpp


)

# ============================================================
# PRIVATE：库只给当前目标 untitled2 使用，不会传递给其他依赖本项目的第三方。
# ============================================================
target_link_libraries(HxnChat
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS HxnChat
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# ============================================================
# 生成部署脚本，内部封装 windeployqt 的逻辑；
# 执行 cmake --install 的时候，会自动运行部署脚本，自动拷贝依赖 Qt dll、插件到输出目录，
# 打包出来可以直接双击 exe 在别的电脑运行，不需要装 Qt 环境。
# 但是只是自动加载qt的dll如果引入三方库需要给exe配置dll来打包
# ============================================================
qt_generate_deploy_app_script(
    TARGET HxnChat
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
