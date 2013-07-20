<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>


<head>
  <META NAME="ROBOTS" CONTENT="INDEX, NOFOLLOW">
  <?-- title --?>
  <?if_total?><title><?slang id="300"/?></title><?/if_total?>
  <?if_daily?><title><?slang id="301"/?></title><?/if_daily?>
  <?if_weekly?><title><?slang id="302"/?></title><?/if_weekly?>
  <?if_monthly?><title><?slang id="303"/?></title><?/if_monthly?>
  <?-- CSS --?><?slang id="1"/?>
</head>



<?-- body tag --?><?slang id="10"/?>

<center>
  <?-- title --?>
  <?if_total?><H1><?slang id="310"/?></H1><?/if_total?>
  <?if_daily?><H1><?slang id="311"/?></H1><?/if_daily?>
  <?if_weekly?><H1><?slang id="312"/?></H1><?/if_weekly?>
  <?if_monthly?><H1><?slang id="313"/?></H1><?/if_monthly?>

<?-- "sorted by whatever" --?><?slang id="320"/?><br>
<?-- "user peak: 123" --?><?slang id="330"/?><br>
<br>
<?template name="toptable"/?>
<br>
<b><?-- "Total users: 123" --?><?slang id="350"/?></b><br>
<br>
<?-- "stats logged since 03.12. 1999 14:11" --?><?slang id="360"/?><br>
<br>
<a href="../../custom/"><?-- "custom sorting" --?><?slang id="370"/?></a><br>
<br>
<a href="../graphs/"><?-- "graphs" --?><?slang id="380"/?></a><br>
<br>
<table width="75%">
  <tr>
    <td width="25%" align="center"><a href="../../total/words/"><?-- "total" --?><?slang id="110"/?></a></td>
    <td width="25%" align="center"><a href="../../daily/words/"><?-- "today" --?><?slang id="111"/?></a></td>
    <td width="25%" align="center"><a href="../../weekly/words/"><?-- "weekly" --?><?slang id="112"/?></a></td>
    <td width="25%" align="center"><a href="../../monthly/words/"><?-- "monthly" --?><?slang id="113"/?></a></td>
  </tr>
</table>
<br>
<?template name="navbar"/?>
<br>

<?template name="credits"/?>
</center>
</body>

</html>