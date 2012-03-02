<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=Shift_JIS">
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
    <b>経路指定編集のヒント</b>
     </font>
  </td>
  </tr>  
  <tr>
    <td class="content_desc_td" width="532">   
    <p><font size="4"><b>エントリの追加と削除</b></font></p>
      <li>
        <b>追加</b> : データを表の最初の行に入力し、<b>追加</b>を押してそのデータを挿入します。システムは、表が一杯になっているか、または必須フィールドが未記入かを指摘します。
      </li> 
      <li>
        <b>削除</b> : 表の1つのエントリを選択するか、<b>Ctrl</b>を押して表の複数のエントリを選択し、<b>Del</b>を押してそれらのエントリを削除します。
      </li> 
    <p><font size="4"><b>入力フォーマット</b></font></p>        
      <p>経路指定規則のフィールドを以下にリストアップします。</p>           
      <li>
        <b>ネットワーク/ホストIP</b> : これは、宛先ネットワークまたは経路指定規則のホストを表します。従って、"192.168.123.11"などのホストアドレス、または"192.168.0.0"などのネットワークアドレスとなることができます。
      </li>       
      <li>
        <b>ネットマスク</b> : これは、どれだけの数のビットがネットワークIDとサブネットIDに対して当てられるかを示しています。例: ドット付き10進表記のネットマスクが255.255.255.0であれば、そのネットマスクビットは24です。宛先がホストであれば、そのネットマスクビットは32になります。
      </li> 
      <li>
        <b>ゲートウェイ</b> : これは、パケットが経路指定されるゲートウェイのIPアドレスを表します。指定されたゲートウェイが最初に到達できる必要があります。つまり、前もってゲートウェイに対してスタティックルートを設定しなければならないことを意味します。
      </li>        
       <li>
        <b>マトリクス</b> : メートルはネットワークの距離の値です
      </li>  
       <li>
        <b>インターフェース</b> : ルートの規則が適用されるネットワークインターフェイス。
      </li>
    <p align="right" style="margin-right:20"><input type="button" onClick="onClose()" value="完了" name="Close1">
        <p></p>
    </td>
  </tr>
</table>
</form>
</body>
</html>
