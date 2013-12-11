ZIPCMD="zip"
echo Compressing Resources
${ZIPCMD} -jqu9 ScriptingTestPlugin.zip resources/* > /dev/null
${ZIPCMD} -jqu9 tmp_ScriptingTestPlugin.zip TestScriptPlugin/* > /dev/null
${ZIPCMD} -jqu9 tmp_ScriptingTestPlugin.zip ScriptingTestPlugin.zip > /dev/null

echo rename files
mv tmp_ScriptingTestPlugin.zip ScriptingTestPlugin.cbplugin

echo deleting temp files
rm ScriptingTestPlugin.zip
