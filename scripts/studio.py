import os
import xml.etree.ElementTree as etree
from mkdist import do_copy_folder, do_copy_file


def update_cproject(cproject_file, bsp_path, project_name):
    # generate cproject
    cproject = etree.parse(cproject_file).getroot()
    configurations = cproject.findall('storageModule/configuration')
    for configuration in configurations:
        resource = configuration.find('resource')
        resource.set('resourceType', "PROJECT")
        resource.set('workspacePath', '/' + project_name)

    # write back to .cproject
    out = open(cproject_file, 'wb')
    out.write('<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'.encode('UTF-8'))
    out.write('<?fileVersion 4.0.0?>'.encode('utf-8'))
    out.write(etree.tostring(cproject, encoding='utf-8'))
    out.close()


def update_project(project_file, bsp_path, project_name):
    # generate project
    project = etree.parse(project_file).getroot()
    project.find("name").text = project_name
    # links = project.findall('linkedResources/link')
    # for link in links:
    #     folder_item = link.find("name").text
    #     if folder_item == "oneos":
    #         continue
    #     folder_name = folder_item.split("/")[-1]
    #     link.find("location").text = os.path.abspath(os.path.join(bsp_path, folder_name))

    out = open(project_file, 'wb')
    out.write('<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'.encode('UTF-8'))
    out.write(etree.tostring(project, encoding='utf-8'))
    out.close()


def update_launch(launch_file, bsp_path, project_name):
    uvprojx = etree.parse(os.path.join(bsp_path, "project.uvprojx")).getroot()
    device = uvprojx.find("Targets/Target/TargetOption/TargetCommonOption/Device").text
    launch = etree.parse(launch_file).getroot()
    for element in launch.findall("stringAttribute"):
        if element.get("key") in ("ilg.gnumcueclipse.debug.gdbjtag.jlink.flashDeviceName",
                                  "ilg.gnumcueclipse.debug.gdbjtag.jlink.gdbServerDeviceName"):
            element.set("value", device)
        if element.get("key") == "org.eclipse.cdt.launch.PROJECT_ATTR":
            element.set("value", project_name)
    for ele in launch.findall("listAttribute"):
        if ele.get("key") == "org.eclipse.debug.core.MAPPED_RESOURCE_PATHS":
            ele.find("listEntry").set("value", "/" + project_name)

    out = open(launch_file, 'wb')
    out.write('<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'.encode('UTF-8'))
    out.write(etree.tostring(launch, encoding='utf-8'))
    out.close()

def init_project(project_path):
    try:
        # 1. copy template
        cproject_file = os.path.join(project_path, ".cproject")
        do_copy_file("../templates/studio/.cproject", cproject_file)
        project_file = os.path.join(project_path, ".project")
        do_copy_file("../templates/studio/.project", project_file)
        setting_path = os.path.join(project_path, ".settings")
        do_copy_folder("../templates/studio/.settings", setting_path)
        # 2. update config
        project_name = os.path.basename(project_path)
        update_cproject(cproject_file, project_path, project_name)
        update_project(project_file, project_path, project_name)
        dst_jlink_launch = os.path.join(setting_path, "%s.JLink.Debug.oneoslaunch" % project_name)
        dst_stlink_launch = os.path.join(setting_path, "%s.STLink.Debug.oneoslaunch" % project_name)
        os.rename(os.path.join(setting_path, "projectName.JLink.Debug.oneoslaunch"), dst_jlink_launch)
        os.rename(os.path.join(setting_path, "projectName.STLink.Debug.oneoslaunch"), dst_stlink_launch)
        update_launch(dst_jlink_launch, project_path, project_name)
        update_launch(dst_stlink_launch, project_path, project_name)
    except Exception as e:
        print(str(e))
