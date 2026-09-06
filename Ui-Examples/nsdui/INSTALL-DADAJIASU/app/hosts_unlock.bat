echo y|cacls %windir%\system32\drivers\etc\hosts /g everyone:f  
attrib -r -a -s -h %windir%\system32\drivers\etc\hosts 