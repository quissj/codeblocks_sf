@echo off

echo Compressing Resources
zip -jqu9 ScriptingTestPlugin.zip resources/*
zip -jqu9 tmp_ScriptingTestPlugin.zip TestScriptPlugin/* 
zip -jqu9 tmp_ScriptingTestPlugin.zip ScriptingTestPlugin.zip

echo rename files
ren tmp_ScriptingTestPlugin.zip ScriptingTestPlugin.cbplugin

echo deleting temp files
del /q ScriptingTestPlugin.zip

