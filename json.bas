Attribute VB_Name = "json导出"
Private Declare Function OpenProcess Lib "kernel32" (ByVal dwDesiredaccess&, ByVal bInherithandle&, ByVal dwProcessid&) As Long
Private Declare Function GetExitCodeProcess Lib "kernel32" (ByVal hProcess As Long, lpexitcode As Long) As Long
Const STILL_ACTIVE = &H103
Const PROCESS_QUERY_INFORMATION = &H400

Const isRun = True

'换行开关
Const vbBr = vbCrLf
'数值错误检测开关
Const checkErr = True

Sub SaveOffice()
    SaveJsonOneFile "官职", "office\office"
    
    
End Sub

Sub SaveMainQuest()
    SaveJsonOneFile "主线任务", "mainquest\mainquest"
    
    
End Sub
Sub SaveQD()
    SaveJson "渠道DATA", "qd\qd_"
End Sub


Public Sub ShellWaitAppLication(ByVal sCommandLine As String)

        On Error GoTo Z
        If Not isRun Then
            GoTo Z
        End If
        Dim hShell     As Long
        Dim hProc     As Long
        Dim lExit     As Long
        hShell = Shell(sCommandLine, vbHide) 'vbhide
        hProc = OpenProcess(&H400, False, hShell)
        Do
                GetExitCodeProcess hProc, lExit
                DoEvents
        Loop While lExit = &H103
Z:
End Sub

Sub Package_all()
    AutoPackage (0)
End Sub

Sub Package_mdpi()
    AutoPackage (1)
End Sub

Sub Package_hdpi()
    AutoPackage (2)
End Sub


'自动打包
Sub AutoPackage(dpi)
    projectPath = "d:\workspace\sanguo\" '项目路径
    sdkToolsPath = "d:\android-sdk-windows\tools\" '安卓工具包路径
    toolsPath = "d:\android-sdk-windows\platform-tools\" '安卓工具包路径
    androidPath = "d:\android-sdk-windows\platforms\android-8\" '安卓平台路径
    javaBinPath = "C:\Program Files\Java\jdk1.6.0_26\bin\"
    logoPath = "d:\logo\"
    
    apkPath = "d:\workspace\" '输出路径
    apkVer = "9300" '输出版本号，用于生成打包文件名
    apkVerStr = "1.4." + apkVer '版本号完整名称
    
    
    
    If dpi = 1 Then
        dpi_path = " -c mdpi,nodpi"
        apkPath = apkPath & "mdpi_"
    ElseIf dpi = 2 Then
        dpi_path = " -c hdpi,nodpi"
        apkPath = apkPath & "hdpi_"
    Else
        dpi_path = " "
        apkPath = apkPath & ""
    End If
    
    
    sheetName = "渠道DATA" '渠道数据表名
    intRow = 3
    
    
    '改版本信息
    varFilePath = projectPath + "AndroidManifest.xml"
    Set fso = CreateObject("scripting.filesystemobject")
    Set fs = fso.OpenTextFile(varFilePath, 1, True)
    xmlData = fs.readall
    fs.Close
    
    spos = InStr(1, xmlData, "android:versionCode=")
    
    sxml = Mid(xmlData, 1, spos - 1)
    sxml = sxml + "android:versionCode=""" + apkVer + """ android:versionName=""" + apkVerStr + """>"
    spos = InStr(1, xmlData, """>") + 2
    eXml = Mid(xmlData, spos, Len(xmlData) - spos + 1)
    
    xmlData = sxml + eXml

    
    
    Set fs = fso.OpenTextFile(varFilePath, 2, True)
    fs.write xmlData
    fs.Close
    
    If dpi = 1 Then
        fso.movefolder projectPath & "assets\c_800x480", "d:\c_800x480"
    ElseIf dpi = 2 Then
        fso.movefolder projectPath & "assets\c_480x320", "d:\c_480x320"
    End If
    
    
    outScript = "#脚本开始" + vbCrLf
    Do Until Sheets(sheetName).Cells(intRow, 1).Value = ""
        If Sheets(sheetName).Cells(intRow, 5).Value = "已生成！" Then
            GoTo ED
        End If
        'MsgBox projectPath + "bin\classes.dex"
        'If fso.FileExists(projectPath + "bin\classes.dex") Then
        '    fso.deletefile projectPath + "bin\classes.dex", True
        '    fso.deletefile projectPath + "bin\resources.ap_", True
        'End If
        jsonData = ""
        For r = 1 To 4
            If jsonData = "" Then
                jsonData = "{"
            Else
                jsonData = jsonData & ","
            End If
            If IsNumeric(Sheets(sheetName).Cells(intRow, r).Value) And Sheets(sheetName).Cells(1, r).Value <> "id" Then
                jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:" & Sheets(sheetName).Cells(intRow, r).Value
            Else
                jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:""" & Sheets(sheetName).Cells(intRow, r).Value & """"
            End If
        Next
        jsonData = jsonData + "}"
        outScript = outScript + vbCrLf + "#生成" & Sheets(sheetName).Cells(intRow, 2).Value & "脚本"

        qdInfoPath = projectPath + "\assets\c_common\qd\qd.json"
        '生成渠道信息
        SaveToFile jsonData, qdInfoPath
        '把LOGO拷贝过去
        If Sheets(sheetName).Cells(intRow, 4).Value = "1" Then
            If fso.FileExists(logoPath + "logo_" + Sheets(sheetName).Cells(intRow, 1).Value + ".png") Then
                fso.copyfile logoPath + "logo_" + Sheets(sheetName).Cells(intRow, 1).Value + ".png", projectPath + "res\drawable-hdpi\qd_logo.png", True
            End If
        End If
        
        Set fs = fso.OpenTextFile(varFilePath, 1, True)
        xmlData = fs.readall
        fs.Close
        
        tmpStr = "android:name=""UMENG_APPKEY""/>"
        spos = InStr(1, xmlData, tmpStr) + Len(tmpStr)
        sxml = Mid(xmlData, 1, spos)

        spos = InStr(spos, xmlData, "</application>")
        eXml = Mid(xmlData, spos, Len(xmlData) - spos + 1)
        
        tmpStr = "<meta-data android:value=""" & Sheets(sheetName).Cells(intRow, 1).Value & """ android:name=""UMENG_CHANNEL""/>" & vbCrLf
        xmlData = sxml + tmpStr + eXml
   
        
        Set fs = fso.OpenTextFile(varFilePath, 2, True)
        fs.write xmlData
        fs.Close

        Sheets(sheetName).Cells(intRow, 5).Value = "10%"
        
        '使用aapt生成R.java类文件
        makeScript = toolsPath + "aapt.exe package -f -m -J " + projectPath + "gen -S " + projectPath + "res -I " + androidPath + "android.jar -M " + projectPath + "AndroidManifest.xml -I " + projectPath + "lib\gson-1.7.1.jar  -I " + projectPath + "lib\AndroidAnalyticSDK3.2.1.jar"
        outScript = outScript + vbCrLf + makeScript
        ShellWaitAppLication (makeScript)
        Sheets(sheetName).Cells(intRow, 5).Value = "30%"

        '第三步 编译.java类文件生成class文件
        makeScript = "javac -encoding utf-8 -target 1.5 -classpath " + projectPath + "lib -bootclasspath " + androidPath + "android.jar -d " + projectPath + "bin  -sourcepath " + projectPath + "src\ " + projectPath + "gen\com\mango\sanguo\R.java"
        ShellWaitAppLication (makeScript)
        outScript = outScript + vbCrLf + makeScript
        Sheets(sheetName).Cells(intRow, 5).Value = "40%"
        '使用android SDK提供的dx.bat命令行脚本生成classes.dex文件,加上第三方的包
        makeScript = "" + toolsPath + "dx.bat --dex --output=" + projectPath + "bin\classes.dex " + projectPath + "bin\classes " + projectPath + "lib\gson-1.7.1.jar " + projectPath + "lib\AndroidAnalyticSDK3.2.1.jar"
        ShellWaitAppLication (makeScript)
        outScript = outScript + vbCrLf + makeScript
        Sheets(sheetName).Cells(intRow, 5).Value = "50%"
        
        
        '使用Android SDK提供的aapt.exe生成资源包文件（包括res、assets、androidmanifest.xml等）
        makeScript = toolsPath + "aapt.exe package -f " + dpi_path + " -M " + projectPath + "AndroidManifest.xml -S " + projectPath + "res -A " + projectPath + "assets -I " + androidPath + "android.jar -F " + projectPath + "bin\resources.ap_ -I " + projectPath + "lib\gson-1.7.1.jar -I " + projectPath + "lib\AndroidAnalyticSDK3.2.1.jar"
        ShellWaitAppLication (makeScript)
        outScript = outScript + vbCrLf + makeScript
        Sheets(sheetName).Cells(intRow, 5).Value = "60%"
        
        '生成未签名的apk安装文件
        apkName = "fytx_" + Sheets(sheetName).Cells(intRow, 1).Value + "_" + apkVer + ".apk"
        makeScript = "" + sdkToolsPath + "apkbuilder.bat " + projectPath + "bin\" + apkName + " -u -z " + projectPath + "bin\resources.ap_ -f " + projectPath + "bin\classes.dex -rf " + projectPath + "src -rj " + projectPath + "lib >>d:\db.log"
       ShellWaitAppLication (makeScript)
        outScript = outScript + vbCrLf + makeScript
        Sheets(sheetName).Cells(intRow, 5).Value = "70%"
        '使用jdk的jarsigner对未签名的包进行apk签名
        makeScript = javaBinPath & "jarsigner -keystore " + sdkToolsPath + "xasg.key -storepass mango123 -keypass mango123 -digestalg SHA1 -sigalg MD5withRSA -signedjar " + projectPath + "bin\signed_" + apkName + " " + projectPath + "bin\" + apkName + " cert"
        ShellWaitAppLication (makeScript)
        outScript = outScript + vbCrLf + makeScript
        Sheets(sheetName).Cells(intRow, 5).Value = "80%"
        '使用android sdk的zipalign工具优化已签名的apk文件
        makeScript = sdkToolsPath + "zipalign -v -f 4 " + projectPath + "bin\signed_" + apkName + " " + apkPath + apkName
        ShellWaitAppLication (makeScript)
        outScript = outScript + vbCrLf + makeScript
        Sheets(sheetName).Cells(intRow, 5).Value = "90%"
        Sheets(sheetName).Cells(intRow, 5).Value = "已生成！"
        If intRow > 4 Then
            'Return
        End If
ED:
        intRow = intRow + 1
    Loop


    If dpi = 1 Then
        fso.movefolder "d:\c_800x480", projectPath & "assets\c_800x480"
    ElseIf dpi = 2 Then
        fso.movefolder "d:\c_480x320", projectPath & "assets\c_480x320"
    End If
    SaveToFile outScript, "d:\db.log"
    Set fs = Nothing
    Set fso = Nothing

End Sub





Sub SaveNPC()
    SaveJson "NPC军团", "corps\team\tm_"
End Sub

Sub SaveNPCWarpath()
    SaveJsonByFileName "NPC军团部队", "corps\troop\tp_"
End Sub


Sub SaveLegionScience()
    SaveJsonOneFile "军团科技数值表", "legion\lg_sc"
End Sub

Sub SaveShop()
    SaveJsonOneFile "商店", "shop\itemlist"
End Sub

Sub SaveMerchant()
    SaveJson "商人", "merchant\mc_"
End Sub

Sub SaveTax()
    SaveJson "征收事件", "tax\ti_"
End Sub

'导出科技JSON数据
Sub SaveScience()
    SaveJson "科技", "science\sc_"
End Sub


Private Sub SaveJson(sheetName, filePath)
    Set fso = CreateObject("scripting.filesystemobject")
    intCol = 1
    intRow = 3
    jsonData = ""
    errData = ""

    Do Until Sheets(sheetName).Cells(1, intCol).Value = ""
    'Range("M" + intLen).Value = ""
      'MsgBox Sheets(sheetName).Cells(1, intCol).Value
      intCol = intCol + 1
    Loop
    
    '保存数据
    folderPath = "e:\data\"
    
    'MsgBox UBound(Split(filePath, "\"))
    'MsgBox Split(filePath, "\")(UBound(Split(filePath, "\")) - 1)
    tmppath = folderPath & Replace(filePath, "\" & Split(filePath, "\")(UBound(Split(filePath, "\"))), "\")
    
    
    If Not fso.FolderExists(tmppath) Then
        fso.CreateFolder tmppath
    End If
            
    Do Until Sheets(sheetName).Cells(intRow, 1).Value = ""
        jsonData = ""
        If Sheets(sheetName).Cells(intRow, 1) <> "" Then
            
            For r = 1 To intCol - 1
              If jsonData = "" Then
                  jsonData = "{"
              Else
                  jsonData = jsonData & "," & vbBr
              End If
              'MsgBox Sheets(sheetName).Cells(intRow, r).Value
              If IsNumeric(Sheets(sheetName).Cells(intRow, r).Value) Then
                t = Trim(CStr(Sheets(sheetName).Cells(intRow, r).Value))
                b = Sheets(sheetName).Cells(intRow, r)
                If t = "True" Or t = "False" Then
                    t = LCase(t)
                End If
                If b = "" And checkErr Then
                    errData = errData & vbBr & "[" & sheetName & "]" & Sheets(sheetName).Cells(intRow, 2).Value & "中属性" & Sheets(sheetName).Cells(1, r).Value & "值为空"
                ElseIf Mid(t, 1, 1) = "." Then
                    t = "0" + t
                End If
                jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:" & t
              Else
                If Mid(Sheets(sheetName).Cells(intRow, r).Value, 1, 1) = "[" Then
                    jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:" & Sheets(sheetName).Cells(intRow, r).Value
                Else
                    jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:""" & Sheets(sheetName).Cells(intRow, r).Value & """"
                End If
              End If
              
            Next r
    
            jsonData = jsonData + "}"

            'Set jsonFile = fso.CreateTextFile(filePath, True)
            'jsonFile.WriteLine (jsonData)
            'jsonFile.Close
            SaveToFile jsonData, folderPath & filePath & Sheets(sheetName).Cells(intRow, 1).Value & ".json"

        End If
        intRow = intRow + 1
    Loop
    
    Set fso = Nothing
    
    MsgBox "操作成功！" & errData
End Sub



Private Sub SaveJsonOneFile(sheetName, filePath)
    Set fso = CreateObject("scripting.filesystemobject")
    intCol = 1
    intRow = 3
    jsonData = ""
    errData = ""

    Do Until Sheets(sheetName).Cells(1, intCol).Value = ""
    'Range("M" + intLen).Value = ""
      'MsgBox Sheets(sheetName).Cells(1, intCol).Value
      intCol = intCol + 1
    Loop
    
    '保存数据
    folderPath = "e:\data\"
    
    'MsgBox UBound(Split(filePath, "\"))
    'MsgBox Split(filePath, "\")(UBound(Split(filePath, "\")) - 1)
    tmppath = folderPath & Replace(filePath, "\" & Split(filePath, "\")(UBound(Split(filePath, "\"))), "\")
    

    If Not fso.FolderExists(tmppath) Then
        fso.CreateFolder tmppath
    End If
            
    Do Until Sheets(sheetName).Cells(intRow, 1).Value = ""

        If Sheets(sheetName).Cells(intRow, 1) <> "" Then
            If jsonData <> "" Then
                jsonData = jsonData & "," & vbBr
            End If
            jsonData = jsonData & "{"
            For r = 1 To intCol - 1
              If r > 1 Then
                  jsonData = jsonData & "," & vbBr
              End If
              'MsgBox Sheets(sheetName).Cells(intRow, r).Value
              If IsNumeric(Sheets(sheetName).Cells(intRow, r).Value) Then
                t = Trim(CStr(Sheets(sheetName).Cells(intRow, r).Value))
                b = Sheets(sheetName).Cells(intRow, r)
                If t = "True" Or t = "False" Then
                    t = LCase(t)
                End If
                If b = "" And checkErr Then
                    errData = errData & vbBr & "[" & sheetName & "]" & Sheets(sheetName).Cells(intRow, 2).Value & "中属性" & Sheets(sheetName).Cells(1, r).Value & "值为空"
                ElseIf Mid(t, 1, 1) = "." Then
                    t = "0" + t
                End If
                jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:" & t
              Else
                If Mid(Sheets(sheetName).Cells(intRow, r).Value, 1, 1) = "[" Then
                    jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:" & Sheets(sheetName).Cells(intRow, r).Value
                Else
                    jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:""" & Sheets(sheetName).Cells(intRow, r).Value & """"
                End If
              End If
              
            Next r
    
            jsonData = jsonData + "}"

        End If
        intRow = intRow + 1
    Loop
    jsonData = "[" & jsonData & "]"
    SaveToFile jsonData, folderPath & filePath & ".json"
    Set fso = Nothing
    
    MsgBox "操作成功！" & errData
End Sub



Private Sub SaveJsonByFileName(sheetName, filePath)
    Set fso = CreateObject("scripting.filesystemobject")
    intCol = 1
    intRow = 5
    jsonData = ""
    errData = ""

    Do Until Sheets(sheetName).Cells(1, intCol).Value = ""
    'Range("M" + intLen).Value = ""
      'MsgBox Sheets(sheetName).Cells(1, intCol).Value
      intCol = intCol + 1
    Loop
    
    '保存数据
    folderPath = "e:\data\"
    
    'MsgBox UBound(Split(filePath, "\"))
    'MsgBox Split(filePath, "\")(UBound(Split(filePath, "\")) - 1)
    tmppath = folderPath & Replace(filePath, "\" & Split(filePath, "\")(UBound(Split(filePath, "\"))), "\")
    

    If Not fso.FolderExists(tmppath) Then
        fso.CreateFolder tmppath
    End If
            
    lastFileName = ""
    Do Until Sheets(sheetName).Cells(intRow, 1).Value = ""
        
        If Sheets(sheetName).Cells(intRow, 1) <> "" Then
            If jsonData <> "" Then
                jsonData = jsonData & "," & vbBr
            End If
            jsonData = jsonData & "{"
            For r = 2 To intCol - 1
              If r > 1 Then
                  jsonData = jsonData & "," & vbBr
              End If
              'MsgBox Sheets(sheetName).Cells(intRow, r).Value
              If IsNumeric(Sheets(sheetName).Cells(intRow, r).Value) Then
                t = Trim(CStr(Sheets(sheetName).Cells(intRow, r).Value))
                b = Sheets(sheetName).Cells(intRow, r)
                If t = "True" Or t = "False" Then
                    t = LCase(t)
                End If
                If b = "" And checkErr Then
                    errData = errData & vbBr & "[" & sheetName & "]" & Sheets(sheetName).Cells(intRow, 2).Value & "中属性" & Sheets(sheetName).Cells(1, r).Value & "值为空"
                ElseIf Mid(t, 1, 1) = "." Then
                    t = "0" + t
                End If
                jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:" & t
              Else
                If Mid(Sheets(sheetName).Cells(intRow, r).Value, 1, 1) = "[" Then
                    jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:" & Sheets(sheetName).Cells(intRow, r).Value
                Else
                    jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:""" & Sheets(sheetName).Cells(intRow, r).Value & """"
                End If
              End If
              
            Next r
    
            jsonData = jsonData + "}"

        End If
        If lastFileName = "" Then
            lastFileName = Sheets(sheetName).Cells(intRow, 1).Value
        End If
        
        If lastFileName <> Sheets(sheetName).Cells(intRow, 1).Value Then
            
            jsonData = "[" & jsonData & "]"
            SaveToFile jsonData, folderPath & filePath & lastFileName & ".json"
            lastFileName = Sheets(sheetName).Cells(intRow, 1).Value
            jsonData = ""
        End If
        intRow = intRow + 1
    Loop
    SaveToFile jsonData, folderPath & filePath & lastFileName & ".json"
    Set fso = Nothing
    
    MsgBox "操作成功！" & errData
End Sub



'导出技能JSON数据
Sub SaveSkill()
    SaveJson "技能", "skill\sk_"
End Sub


'导出建筑JSON数据
Sub SaveBuild()
    SaveJson "建筑", "build\build_"
End Sub

'导出武将JSON数据
Sub SaveGen()
Attribute SaveGen.VB_ProcData.VB_Invoke_Func = " \n14"

    Set fso = CreateObject("scripting.filesystemobject")
    intCol = 1
    intRow = 3
    jsonData = ""
    errData = ""
    sheetName = "武将"
    Do Until Sheets(sheetName).Cells(1, intCol).Value = ""
    'Range("M" + intLen).Value = ""
      'MsgBox Sheets(sheetName).Cells(1, intCol).Value
      intCol = intCol + 1
    Loop
    
    Do Until Sheets(sheetName).Cells(intRow, 1).Value = ""
        jsonData = ""
        If Sheets(sheetName).Cells(intRow, 2) <> "" Then
            
            For r = 1 To intCol - 1
              If jsonData = "" Then
                  jsonData = "{"
              Else
                  jsonData = jsonData & "," & vbBr
              End If
              'MsgBox Sheets(sheetName).Cells(intRow, r).Value
              If IsNumeric(Sheets(sheetName).Cells(intRow, r).Value) Then
                t = Trim(CStr(Sheets(sheetName).Cells(intRow, r).Value))
                b = Sheets(sheetName).Cells(intRow, r)
                If b = "" And checkErr Then
                    errData = errData & vbBr & "[sheetName]" & Sheets(sheetName).Cells(intRow, 1).Value & "中属性" & Sheets(sheetName).Cells(1, r).Value & "值为空"
                ElseIf Mid(t, 1, 1) = "." Then
                    t = "0" + t
                End If
                jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:" & t
              Else
                jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:""" & Sheets(sheetName).Cells(intRow, r).Value & """"
              End If
              
            Next r
    
            jsonData = jsonData + "}"
            '保存数据
            folderPath = "e:\data\general"
            If Not fso.FolderExists(folderPath) Then
                fso.CreateFolder folderPath
            End If
            filePath = folderPath & "\gen_" & Sheets(sheetName).Cells(intRow, 2).Value & ".json"
    
            'Set jsonFile = fso.CreateTextFile(filePath, True)
            'jsonFile.WriteLine (jsonData)
            'jsonFile.Close
            SaveToFile jsonData, filePath

        End If
        intRow = intRow + 1
    Loop
    
    Set fso = Nothing
    
    MsgBox "操作成功！" & errData
End Sub

'导出城市JSON数据
Sub SaveCity()
    SaveJson "城市", "city\ct_"
End Sub


'导出装备JSON数据
Sub SaveEqu()
    SaveJson "装备", "equipment\eq_"
End Sub

'导出兵种JSON数据
Sub SaveSoldier()

    Set fso = CreateObject("scripting.filesystemobject")
    intCol = 1
    intRow = 3
    jsonData = ""
    errData = ""
    sheetName = "兵种"
    Do Until Sheets(sheetName).Cells(1, intCol).Value = ""
    'Range("M" + intLen).Value = ""
      'MsgBox Sheets(sheetName).Cells(1, intCol).Value
      intCol = intCol + 1
    Loop
    
    Do Until Sheets(sheetName).Cells(intRow, 1).Value = ""
        jsonData = ""
        
        If Sheets(sheetName).Cells(intRow, 2) <> "" Then
        
        
            For r = 1 To intCol - 1
              If jsonData = "" Then
                  jsonData = "{"
              Else
                  jsonData = jsonData & "," & vbBr
              End If
              'MsgBox Sheets(sheetName).Cells(intRow, r).Value
              If IsNumeric(Sheets(sheetName).Cells(intRow, r).Value) Then
                t = Trim(CStr(Sheets(sheetName).Cells(intRow, r).Value))
                If t = "" And checkErr Then
                    errData = errData & vbBr & "[sheetName]" & Sheets(sheetName).Cells(intRow, 1).Value & " 属性" & Sheets(sheetName).Cells(1, r).Value & "值为空"
                ElseIf Mid(t, 1, 1) = "." Then
                    t = "0" + t
                End If
                jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:" & t
              Else
                jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:""" & Sheets(sheetName).Cells(intRow, r).Value & """"
              End If
              
            Next r
    
            jsonData = jsonData + "}"
            '保存数据
            folderPath = "e:\data\soldier"
            If Not fso.FolderExists(folderPath) Then
                fso.CreateFolder folderPath
            End If
            filePath = folderPath & "\soldier_" & Sheets(sheetName).Cells(intRow, 2).Value & ".json"
    
            'Set jsonFile = fso.CreateTextFile(filePath, True)
            'jsonFile.WriteLine (jsonData)
            'jsonFile.Close
            
            SaveToFile jsonData, filePath
            
        End If
        intRow = intRow + 1
    Loop

    Set fso = Nothing
    MsgBox "操作成功！" & errData
End Sub


'导出部队JSON数据
Sub SaveWarpathMap()

    Set fso = CreateObject("scripting.filesystemobject")
    
    intCol = 1
    intRow = 3
    jsonData = ""
    listData = ""
    errData = ""
    
    headerData = ""
    
    sheetName = "部队"
    Do Until Sheets(sheetName).Cells(1, intCol).Value = ""
    'Range("M" + intLen).Value = ""
      'MsgBox Sheets(sheetName).Cells(1, intCol).Value
      intCol = intCol + 1
    Loop
    For r = 1 To intCol - 1
        If headerData = "" Then
            headerData = "{"
        Else
            headerData = headerData & "," & vbBr
        End If
        If IsNumeric(Sheets(sheetName).Cells(intRow, r).Value) Or Sheets(sheetName).Cells(intRow, r).Value = "null" Or Mid(Sheets(sheetName).Cells(intRow, r).Value, 1, 1) = "[" Then
          t = Trim(CStr(Sheets(sheetName).Cells(intRow, r).Value))
          If t = "" And checkErr Then
              errData = errData & vbBr & "[sheetName]" & Sheets(sheetName).Cells(intRow, 2).Value & " 属性" & Sheets(sheetName).Cells(1, r).Value & "值为空"
          ElseIf Mid(t, 1, 1) = "." Then
              t = "0" + t
          End If
          headerData = headerData & """" + Sheets(sheetName).Cells(1, r).Value & """:" & t
        Else
          headerData = headerData & """" + Sheets(sheetName).Cells(1, r).Value & """:""" & Sheets(sheetName).Cells(intRow, r).Value & """"
        End If
        
    Next r
    intRow = 9
    intCol = 1
    Do Until Sheets(sheetName).Cells(5, intCol).Value = "fid"
    'Range("M" + intLen).Value = ""
      'MsgBox Sheets(sheetName).Cells(1, intCol).Value
      intCol = intCol + 1
    Loop
    
    
    Do Until Sheets(sheetName).Cells(intRow, 1).Value = ""
        jsonData = ""
        
            For r = 1 To intCol - 1
              If jsonData = "" Then
                  jsonData = "{"
              Else
                  jsonData = jsonData & "," & vbBr
              End If
              'MsgBox Sheets(sheetName).Cells(intRow, r).Value
              If IsNumeric(Sheets(sheetName).Cells(intRow, r).Value) Then
                t = Trim(CStr(Sheets(sheetName).Cells(intRow, r).Value))
                If t = "" And checkErr Then
                    errData = errData & vbBr & "[sheetName]" & Sheets(sheetName).Cells(intRow, 2).Value & " 属性" & Sheets(sheetName).Cells(5, r).Value & "值为空"
                ElseIf Mid(t, 1, 1) = "." Then
                    t = "0" + t
                End If
                jsonData = jsonData & """" + Sheets(sheetName).Cells(5, r).Value & """:" & t
              Else
                jsonData = jsonData & """" + Sheets(sheetName).Cells(5, r).Value & """:""" & Sheets(sheetName).Cells(intRow, r).Value & """"
              End If
              
            Next r
            jsonData = jsonData & ",""trd"":[" & vbBr
                
            childRow = intCol
            childList = ""
            childData = ""
            childFid = ""
            '处理小队
            Do Until Sheets(sheetName).Cells(5, childRow) = ""
                  If Sheets(sheetName).Cells(5, childRow).Value = "fid" Then
                    childFid = Sheets(sheetName).Cells(intRow, childRow)
                    
                    If childFid = "" Then
                        childRow = childRow + 14
                    End If
                    
                    If childData <> "" Then
                        If childList <> "" Then
                            childList = childList & "," & vbBr
                        End If
                        childList = childList & childData & "}" & vbBr
                    End If

                    childData = ""
                  End If
                  If childFid <> "" Then
                      If childData = "" Then
                          childData = "{" & vbBr
                      Else
                          childData = childData & "," & vbBr
                      End If
    
                      'MsgBox Sheets(sheetName).Cells(intRow, r).Value
                      If IsNumeric(Sheets(sheetName).Cells(intRow, childRow).Value) Then
                        t = Trim(CStr(Sheets(sheetName).Cells(intRow, childRow).Value))
                        If t = "" Then
                            errData = errData & vbBr & "[sheetName]" & Sheets(sheetName).Cells(intRow, 2).Value & " 属性" & Sheets(sheetName).Cells(5, childRow).Value & "值为空"
                        ElseIf Mid(t, 1, 1) = "." Then
                            t = "0" + t
                        End If
                        childData = childData & """" + Sheets(sheetName).Cells(5, childRow).Value & """:" & t
                      Else
                        childData = childData & """" + Sheets(sheetName).Cells(5, childRow).Value & """:""" & Sheets(sheetName).Cells(intRow, childRow).Value & """"
                      End If
                End If
                childRow = childRow + 1
            Loop
            
            If childData <> "" Then
                If childList <> "" Then
                    childList = childList & ","
                End If
                childList = childList & childData & "}" & vbBr
            End If
            
            jsonData = jsonData & childList & "]}" & vbBr
            
            
        If listData <> "" Then
            listData = listData & "," & vbBr
        End If
        listData = listData & jsonData
        intRow = intRow + 1
    Loop
    
    jsonData = headerData & ",""armyDatas"": [" & listData & "]}"

    '保存数据
    folderPath = "e:\data\warpathMap"
    If Not fso.FolderExists(folderPath) Then
        fso.CreateFolder folderPath
    End If
    filePath = folderPath & "\wm_" & Sheets(sheetName).Cells(3, 1).Value & ".json"

    'Set jsonFile = fso.CreateTextFile(filePath, True)
    'jsonFile.WriteLine (jsonData)
    'jsonFile.Close

    Set fso = Nothing
    SaveToFile jsonData, filePath
    MsgBox "操作成功！" & errData
End Sub



Sub SaveNpcFieldMap()

    Set fso = CreateObject("scripting.filesystemobject")
    
    intCol = 1
    intRow = 5
    jsonData = ""
    listData = ""
    errData = ""
    sheetName = "田矿NPC部队"
    
        '保存数据
    folderPath = "e:\data\"
    filePath = "res\resNpc_"
    tmppath = folderPath & Replace(filePath, "\" & Split(filePath, "\")(UBound(Split(filePath, "\"))), "\")
    
    
    If Not fso.FolderExists(tmppath) Then
        fso.CreateFolder tmppath
    End If
    
    
    Do Until Sheets(sheetName).Cells(1, intCol).Value = "fid"
    'Range("M" + intLen).Value = ""
      'MsgBox Sheets(sheetName).Cells(1, intCol).Value
      intCol = intCol + 1
    Loop
    
    
    lastFileName = ""
    
    Do Until Sheets(sheetName).Cells(intRow, 1).Value = ""
        jsonData = ""
        
            For r = 1 To intCol - 1
              If jsonData = "" Then
                  jsonData = "{"
              Else
                  jsonData = jsonData & "," & vbBr
              End If
              'MsgBox Sheets(sheetName).Cells(intRow, r).Value
              If IsNumeric(Sheets(sheetName).Cells(intRow, r).Value) Then
                t = Trim(CStr(Sheets(sheetName).Cells(intRow, r).Value))
                If t = "" And checkErr Then
                    errData = errData & vbBr & "[sheetName]" & Sheets(sheetName).Cells(intRow, 2).Value & " 属性" & Sheets(sheetName).Cells(1, r).Value & "值为空"
                ElseIf Mid(t, 1, 1) = "." Then
                    t = "0" + t
                End If
                jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:" & t
              Else
                jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:""" & Sheets(sheetName).Cells(intRow, r).Value & """"
              End If
              
            Next r
            jsonData = jsonData & ",""trd"":[" & vbBr
                
            childRow = intCol
            childList = ""
            childData = ""
            childFid = ""
            '处理小队
            Do Until Sheets(sheetName).Cells(1, childRow) = ""
                  If Sheets(sheetName).Cells(1, childRow).Value = "fid" Then
                    childFid = Sheets(sheetName).Cells(intRow, childRow)
                    
                    If childFid = "" Then
                        childRow = childRow + 14
                    End If
                    
                    If childData <> "" Then
                        If childList <> "" Then
                            childList = childList & "," & vbBr
                        End If
                        childList = childList & childData & "}" & vbBr
                    End If

                    childData = ""
                  End If
                  If childFid <> "" Then
                      If childData = "" Then
                          childData = "{" & vbBr
                      Else
                          childData = childData & "," & vbBr
                      End If
    
                      'MsgBox Sheets(sheetName).Cells(intRow, r).Value
                      If IsNumeric(Sheets(sheetName).Cells(intRow, childRow).Value) Then
                        t = Trim(CStr(Sheets(sheetName).Cells(intRow, childRow).Value))
                        If t = "" Then
                            errData = errData & vbBr & "[sheetName]" & Sheets(sheetName).Cells(intRow, 2).Value & " 属性" & Sheets(sheetName).Cells(1, childRow).Value & "值为空"
                        ElseIf Mid(t, 1, 1) = "." Then
                            t = "0" + t
                        End If
                        childData = childData & """" + Sheets(sheetName).Cells(1, childRow).Value & """:" & t
                      Else
                        childData = childData & """" + Sheets(sheetName).Cells(1, childRow).Value & """:""" & Sheets(sheetName).Cells(intRow, childRow).Value & """"
                      End If
                End If
                childRow = childRow + 1
            Loop
            
            If childData <> "" Then
                If childList <> "" Then
                    childList = childList & ","
                End If
                childList = childList & childData & "}" & vbBr
            End If
            
            jsonData = jsonData & childList & "]}" & vbBr
            
            
            If lastFileName = "" Then
                lastFileName = Sheets(sheetName).Cells(intRow, 1).Value
            End If
        
            
            If lastFileName <> Sheets(sheetName).Cells(intRow, 1).Value Then
                'MsgBox folderPath & filePath & Sheets(sheetName).Cells(intRow, 1).Value
                listData = "[" & listData & "]"
                SaveToFile listData, folderPath & filePath & lastFileName & ".json"
                lastFileName = Sheets(sheetName).Cells(intRow, 1).Value
                listData = jsonData
            Else
                If listData <> "" Then
                    listData = listData & "," & vbBr
                End If
                listData = listData & jsonData
            End If
        
        intRow = intRow + 1
    Loop
    SaveToFile listData, folderPath & filePath & lastFileName & ".json"

    Set fso = Nothing
    
    MsgBox "操作成功！" & errData
End Sub



Sub SaveNpcWarpathMap()

    Set fso = CreateObject("scripting.filesystemobject")
    
    intCol = 1
    intRow = 5
    jsonData = ""
    listData = ""
    errData = ""
    sheetName = "NPC军团部队"
    
        '保存数据
    folderPath = "e:\data\"
    filePath = "corps\troop\tp_"
    tmppath = folderPath & Replace(filePath, "\" & Split(filePath, "\")(UBound(Split(filePath, "\"))), "\")
    
    
    If Not fso.FolderExists(tmppath) Then
        fso.CreateFolder tmppath
    End If
    
    
    Do Until Sheets(sheetName).Cells(1, intCol).Value = "fid"
    'Range("M" + intLen).Value = ""
      'MsgBox Sheets(sheetName).Cells(1, intCol).Value
      intCol = intCol + 1
    Loop
    
    
    lastFileName = ""
    
    Do While Sheets(sheetName).Cells(intRow, 1).Value <> ""
        jsonData = ""
        
            For r = 2 To intCol - 1
              If jsonData = "" Then
                  jsonData = "{"
              Else
                  jsonData = jsonData & "," & vbBr
              End If
              'MsgBox Sheets(sheetName).Cells(intRow, r).Value
              If IsNumeric(Sheets(sheetName).Cells(intRow, r).Value) Then
                t = Trim(CStr(Sheets(sheetName).Cells(intRow, r).Value))
                If t = "" And checkErr Then
                    errData = errData & vbBr & "[sheetName]" & Sheets(sheetName).Cells(intRow, 2).Value & " 属性" & Sheets(sheetName).Cells(1, r).Value & "值为空"
                ElseIf Mid(t, 1, 1) = "." Then
                    t = "0" + t
                End If
                jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:" & t
              Else
                jsonData = jsonData & """" + Sheets(sheetName).Cells(1, r).Value & """:""" & Sheets(sheetName).Cells(intRow, r).Value & """"
              End If
              
            Next r
            jsonData = jsonData & ",""trd"":[" & vbBr
                
            childRow = intCol
            childList = ""
            childData = ""
            childFid = ""
            '处理小队
            Do Until Sheets(sheetName).Cells(1, childRow) = ""
                  If Sheets(sheetName).Cells(1, childRow).Value = "fid" Then
                    childFid = Sheets(sheetName).Cells(intRow, childRow)
                    
                    If childFid = "" Then
                        childRow = childRow + 14
                    End If
                    
                    If childData <> "" Then
                        If childList <> "" Then
                            childList = childList & "," & vbBr
                        End If
                        childList = childList & childData & "}" & vbBr
                    End If

                    childData = ""
                  End If
                  If childFid <> "" Then
                      If childData = "" Then
                          childData = "{" & vbBr
                      Else
                          childData = childData & "," & vbBr
                      End If
    
                      'MsgBox Sheets(sheetName).Cells(intRow, r).Value
                      If IsNumeric(Sheets(sheetName).Cells(intRow, childRow).Value) Then
                        t = Trim(CStr(Sheets(sheetName).Cells(intRow, childRow).Value))
                        If t = "" Then
                            errData = errData & vbBr & "[sheetName]" & Sheets(sheetName).Cells(intRow, 2).Value & " 属性" & Sheets(sheetName).Cells(1, childRow).Value & "值为空"
                        ElseIf Mid(t, 1, 1) = "." Then
                            t = "0" + t
                        End If
                        childData = childData & """" + Sheets(sheetName).Cells(1, childRow).Value & """:" & t
                      Else
                        childData = childData & """" + Sheets(sheetName).Cells(1, childRow).Value & """:""" & Sheets(sheetName).Cells(intRow, childRow).Value & """"
                      End If
                End If
                childRow = childRow + 1
            Loop
            
            If childData <> "" Then
                If childList <> "" Then
                    childList = childList & ","
                End If
                childList = childList & childData & "}" & vbBr
            End If
            
            jsonData = jsonData & childList & "]}" & vbBr
            
            

        
        If lastFileName = "" Then
            lastFileName = Sheets(sheetName).Cells(intRow, 1).Value
        End If
        
        If lastFileName <> Sheets(sheetName).Cells(intRow, 1).Value Then
            'MsgBox folderPath & filePath & Sheets(sheetName).Cells(intRow, 1).Value
            listData = "[" & listData & "]"
            SaveToFile listData, folderPath & filePath & lastFileName & ".json"
            lastFileName = Sheets(sheetName).Cells(intRow, 1).Value
            listData = jsonData
        Else
            If listData <> "" Then
                listData = listData & "," & vbBr
            End If
            listData = listData & jsonData
        End If
        
        
        intRow = intRow + 1
    Loop
    SaveToFile listData, folderPath & filePath & lastFileName & ".json"

    Set fso = Nothing
    
    MsgBox "操作成功！" & errData
End Sub

Sub SaveToFile(Content, FileName)

    Dim stm: Set stm = CreateObject("adodb.stream")

    stm.Type = 2 '以文本模式读取

    stm.Mode = 3

    stm.Charset = "utf-8"

    stm.Open

    stm.Writetext (Content)

    stm.Position = 3

    Dim newStream: Set newStream = CreateObject("adodb.stream")

    With newStream

        .Mode = 3

        .Type = 1

        .Open

    End With

    stm.CopyTo (newStream)

    newStream.SaveToFile FileName, 2

    stm.flush

    stm.Close

    Set stm = Nothing

    Set newStream = Nothing

End Sub
