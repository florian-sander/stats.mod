<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
  <META NAME="ROBOTS" CONTENT="INDEX, NOFOLLOW">
  <?-- CSS --?><?slang id="1"/?>
  <title><?slang id="600"/?></title>
</head>


<?-- body tag --?><?slang id="10"/?>
<br>
<center>
<table border="0">
  <tr><th colspan="3"><?-- Currently on channel: --?><?slang id="610"/?></th></tr>
  <tr>
    <th align="center">nick</th>
    <th align="center">user</th>
    <th align="center">info</th>
    <th align="center">idle time</th>
  </tr>
<?init_colorfade steps="onchan"/?>
<?onchanlist?>
  <tr bgcolor="<?fcolor/?>">
    <td align="left"><?usermode/?><?nick/?></td>
    <?if_user?>
       <td align="left">
         <a href="../users/<?user encode="yes"/?>/"><?user/?></a>
       </td>
       <td>
         <?if_icqnr?><img src="http://wwp.icq.com/scripts/online.dll?icq=<?icqnr/?>&amp;img=5" alt="<?icqnr/?>" width="18" height="18"><?/if_icqnr?>
         <?if_binary?>
           <?if_homepage?><a href="<?homepage/?>"><img src="<?binary_url/?>/homepage.gif" height="15" width="15" alt="[URL]" border="0"></a><?/if_homepage?>
           <?if_email?><a href="mailto:<?email/?>"><img src="<?binary_url/?>/email.gif" height="15" width="15" alt="[eMail]" border="0"></a><?/if_email?>
         <?/if_binary?>
       </td>
    <?/if_user?>
    <?if_nouser?> <td align="left"> - </td><td></td><?/if_nouser?>
    <td align="right"><?if_netsplitted?><?slang id="630"/?><?/if_netsplitted?><?idletime/?></td>
  </tr>
  <?fade_color/?>
<?/onchanlist?>
</table>
<br>
<?template name="navbar"/?>
<br>
<?template name="credits"/?>
</center>
</body>

</html>