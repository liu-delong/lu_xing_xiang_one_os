import sys
import glob  
import os.path 
import re
from build_tools import *

insert_string = '<FilePath>..\\..\\arch\\arm\\cortex-m33\\context_rvds.S</FilePath>\n' + \
'              <FileOption>\n\
                <CommonProperty>\n\
                  <UseCPPCompiler>2</UseCPPCompiler>\n\
                  <RVCTCodeConst>0</RVCTCodeConst>\n\
                  <RVCTZI>0</RVCTZI>\n\
                  <RVCTOtherData>0</RVCTOtherData>\n\
                  <ModuleSelection>0</ModuleSelection>\n\
                  <IncludeInBuild>2</IncludeInBuild>\n\
                  <AlwaysBuild>2</AlwaysBuild>\n\
                  <GenerateAssemblyFile>2</GenerateAssemblyFile>\n\
                  <AssembleAssemblyFile>2</AssembleAssemblyFile>\n\
                  <PublicsOnly>2</PublicsOnly>\n\
                  <StopOnExitCode>11</StopOnExitCode>\n\
                  <CustomArgument></CustomArgument>\n\
                  <IncludeLibraryModules></IncludeLibraryModules>\n\
                  <ComprImg>1</ComprImg>\n\
                </CommonProperty>\n\
                <FileArmAds>\n\
                  <Aads>\n\
                    <interw>2</interw>\n\
                    <Ropi>2</Ropi>\n\
                    <Rwpi>2</Rwpi>\n\
                    <thumb>2</thumb>\n\
                    <SplitLS>2</SplitLS>\n\
                    <SwStkChk>2</SwStkChk>\n\
                    <NoWarn>2</NoWarn>\n\
                    <uSurpInc>2</uSurpInc>\n\
                    <useXO>2</useXO>\n\
                    <ClangAsOpt>4</ClangAsOpt>\n\
                    <VariousControls>\n\
                      <MiscControls></MiscControls>\n\
                      <Define></Define>\n\
                      <Undefine></Undefine>\n\
                      <IncludePath></IncludePath>\n\
                    </VariousControls>\n\
                  </Aads>\n\
                </FileArmAds>\n\
              </FileOption>'

find_string = "<UseCPPCompiler>2</UseCPPCompiler>"
def modify_compiler_context_rvds(prj_path, bsp_path):
    source = prj_path + "/project.uvprojx"
    
    f1 = open(source, 'r+')

    content = f1.read()
    find_result = content.find(find_string)
    #print(find_result)
    if find_result < 0:
        content = content.replace("<FilePath>..\\..\\arch\\arm\\cortex-m33\\context_rvds.S</FilePath>", insert_string)
        f1 = open(source, 'w+')
        f1.write(content)
    f1.close()
    
def posttarget(prj_path, bsp_path = ''):
    print("project " + prj_path)
    modify_compiler_context_rvds(prj_path, bsp_path)
    