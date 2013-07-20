<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
  <META NAME="ROBOTS" CONTENT="INDEX, NOFOLLOW">
  <?-- CSS --?><?slang id="1"/?>
  <?-- title --?>
  <?if_total?> <title><?slang id="400"/?></title><?/if_total?>
  <?if_daily?> <title><?slang id="401"/?></title><?/if_daily?>
  <?if_weekly?> <title><?slang id="402"/?></title><?/if_weekly?>
  <?if_monthly?> <title><?slang id="403"/?></title><?/if_monthly?>
</head>


<?-- body tag --?><?slang id="10"/?>

<center>

<?-- title --/?>
<?if_total?><H1><?slang id="410"/?></H1><?/if_total?>
<?if_daily?><H1><?slang id="411"/?></H1><?/if_daily?>
<?if_weekly?><H1><?slang id="412"/?></H1><?/if_weekly?>
<?if_monthly?><H1><?slang id="413"/?></H1><?/if_monthly?>

<br>

<?graphstats?>

<table width="100%" border="0">
  <tr>
  	<!-- <td width="33%"><table border="1"><tr><td width="100%" bgcolor="#6B54C0">&nbsp;</td></tr></table></td> //-->
  	<th align="center" valign="center">
  		<nobr><img src="<?binary_url/?>/6500E1.png" width="100" height="2">&nbsp;&nbsp;<font size="+1"><?-- Ordered by <b>words</b> --?><?slang id="420"/?></font>&nbsp;&nbsp;<img src="<?binary_url/?>/6500E1.png" width="100" height="2"></nobr>
  	</th>
  	<!-- <td width="33%"><table><tr><td bgcolor="#6B54C0">&nbsp;</td></tr></table></td> //-->
  </tr>
</table>
<font size="-1"><?-- [2392920 words total] --?><?slang id="430"/?></font><br>
<br>

<table border="0">

  <tr>
  	<?graphs?>
  	<td align="center">
  		<font size="-1"><?value/?></font><br>
  		<font size="-2"><?graph_percent/?>%</font>
  	</td>
  	<?/graphs?>
  </tr>
  <tr>
  	<?graphs?>
  	<td align="center" valign="bottom"><img src="<?binary_url/?>/vertical_blue_bar.gif" width="10" height="<?graph_percent stretch="true" max="150"/?>" alt=""></td>
  	<?/graphs?>
  </tr>
  <tr>
  	<?graphs?>
  	<td align="center"><font color="#B2B2B2">- <a href="../../../users/<?user encode="yes"/?>/"><?user/?></a> -</font></td>
  	<?/graphs?>
  </tr>

</table>

<br>

<?/graphstats?>
<br>
<table width="75%">
  <tr>
    <td width="25%" align="center"><a href="../../total/graphs/"><?-- "total" --?><?slang id="110"/?></a></td>
    <td width="25%" align="center"><a href="../../daily/graphs/"><?-- "today" --?><?slang id="111"/?></a></td>
    <td width="25%" align="center"><a href="../../weekly/graphs/"><?-- "weekly" --?><?slang id="112"/?></a></td>
    <td width="25%" align="center"><a href="../../monthly/graphs/"><?-- "monthly" --?><?slang id="113"/?></a></td>
  </tr>
</table>
<br>
<?template name="navbar"/?>
<br>
<?template name="credits"/?>
</center>
</body>

</html>