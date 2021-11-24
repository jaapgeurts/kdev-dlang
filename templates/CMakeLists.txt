project(kdevelop_templates)

set(FILE_TEMPLATE_DIRS
    dlang_file_empty)

set(APP_TEMPLATE_DIRS
    dlang_dub_json
    dlang_dub_sdl)


kdevplatform_add_file_templates(${FILE_TEMPLATE_DIRS})

kdevplatform_add_app_templates(${APP_TEMPLATE_DIRS})
