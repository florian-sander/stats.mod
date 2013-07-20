<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
  <?-- CSS --?><?slang id="1"/?>
  <title><?slang id="100"/?></title>
</head>


<?-- body tag --?><?slang id="10"/?>
<br>
<br>
<br>
<center>
<table border="1" width="75%">
<?chanlist?>
  <tr>
    <td rowspan="7" align="center" width="50%">
      <a href="<?chan encode="yes" short="yes"/?>/"><?chan/?></a><br>
      <font size="-1"><?current_topic/?></font>
    </td>
    <td rowspan="4" align="center"><?-- "top 30" --?><?slang id="105"/?></td>
    <td align="center"><a href="<?chan encode="yes" short="yes"/?>/top/total/words/"><?-- "total" --?><?slang id="110"/?></a></td>
  </tr>
  <tr><td align="center"><a href="<?chan encode="yes" short="yes"/?>/top/daily/words/"><?-- "today" --?><?slang id="111"/?></a></td></tr>
  <tr><td align="center"><a href="<?chan encode="yes" short="yes"/?>/top/weekly/words/"><?-- "weekly" --?><?slang id="112"/?></a></td></tr>
  <tr><td align="center"><a href="<?chan encode="yes" short="yes"/?>/top/monthly/words/"><?-- "monthly" --?><?slang id="113"/?></a></td></tr>
  <tr><td align="center" colspan="2"><a href="<?chan encode="yes" short="yes"/?>/users/"><?-- "userlist" --?><?slang id="120"/?></a></td></tr>
  <tr><td align="center" colspan="2"><a href="<?chan encode="yes" short="yes"/?>/onchan/"><?-- "who's on now?" --?><?slang id="121"/?></a></td></tr>
  <tr><td align="center" colspan="2"><a href="<?chan encode="yes" short="yes"/?>/misc/"><?-- "misc stats" --?><?slang id="122"/?></a></td></tr>
  <tr><td colspan="3" height="0"><font size="-5">&nbsp;</font></td></tr>
<?/chanlist?>
</table>
</center>
<?template name="credits"/?>
</body>

</html>