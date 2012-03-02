<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=euc-kr">
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<meta name="GENERATOR" content="Microsoft FrontPage 4.0">
<meta name="ProgId" content="FrontPage.Editor.Document">
<title>Route Edit Tips</title>
<link rel="stylesheet" type="text/css" href="style.css" media="screen">

<SCRIPT LANGUAGE="JavaScript">

function loadWidzard()
{
   
}

function functionChange(o)
{   
     
}

function optionChange(o)
{      
   
}

function onClose()
{
    window.close();
}

</script>

</head>

<body  onLoad="loadWidzard()">
<form method="POST" name="form" action="--WEBBOT-SELF--" ">  
<!--webbot bot="SaveResults" U-File="fpweb:///_private/form_results.txt"
S-Format="TEXT/CSV" S-Label-Fields="TRUE" -->  
<table border="1" width="542">
  <tr>
    <td class="content_header_td_title"  width="532"><font face="Arial" size="5" color="#FFFFFF">
    <b>라우터 편집 도움말</b>
     </font>
  </td>
  </tr>  
  <tr>
    <td class="content_desc_td" width="532">   
    <p><font size="4"><b>입력 추가/삭제</b></font></p>
      <li>
        <b>추가</b> : 데이터를 표의 첫 번째 행에 입력한 후 <b>추가</b>를 눌러 삽입합니다. 시스템이 표가 전부 작성되어 있는 지 또는 필수항목이 작성되었는지의 여부를 알려 줍니다.
      </li> 
      <li>
        <b>삭제</b> : 표에서 한 가지 입력을 하거나 <b>Ctrl</b>를 눌러 표에서 여러 가지 입력을 하고 <b>Del</b>를 눌러 삭제합니다.
      </li> 
    <p><font size="4"><b>입력형식</b></font></p>        
      <p>라우터 규칙이 아래에 열거되어 있음:</p>           
      <li>
        <b>네트워크/호스트IP</b> : 목적지 네트워크 또는 route roule의 호스트를 나타냅니다.  "192.168.123.11"와 같은 호스트 주소나 "192.168.0.0"와 같은 네트워크 주소일 수 있습니다.
      </li>       
      <li>
        <b>넷마스크</b> : 네트워크 ID 또는 서브넷 ID 비트를 나타냅니다. 예: 십진수 넷마스크가 255.255.255.0이면 넷마스크 비트는 24입니다. 목적지가 호스트이면 목적지의 넷마스크 비트는 32이어야 합니다.
      </li> 
      <li>
        <b>게이트웨이</b> : 패킷 라우팅되는 게이트웨이의 IP주소를 나타냅니다. 먼저 특정한 게이트웨이에 도달할 수 있어야 합니다. 이는 사전에 게이트웨이 고정 라우팅을 설정해야 함을 의미합니다.
      </li>        
       <li>
        <b>메트릭</b> : Metric은 네트워크를 위한 거리의 값입니다.
      </li>  
       <li>
        <b>인터페이스</b> : Network interface에서 다양한 적용이 가능합니다.
      </li>
    <p align="right" style="margin-right:20"><input type="button" onClick="onClose()" value="닫기" name="Close1">
        <p></p>
    </td>
  </tr>
</table>
</form>
</body>
</html>
