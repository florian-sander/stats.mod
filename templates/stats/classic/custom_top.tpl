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


<form action="./" method="GET">
<table>
<tr>
	<td>
	Type: <select name="sorting" value="<?type/?>" size="1">
	<?types?>
		<option><?type/?>
	<?/types?>
	</td>
	<td>Places <input type="text" size="3" name="start"> to <input type="text" size="3" name="end">.</td>
	<td><input type="submit" value="List!"></td>
</tr><tr>
	<td colspan="2">
	<table><tr>
		<td>Timerange:</td>
		<td><input type="radio" name="timerange" value="today" <?if_daily?>checked<?/if_daily?>>Today</td>
		<td><input type="radio" name="timerange" value="weekly" <?if_weekly?>checked<?/if_weekly?>>Weekly</td>
		</td>
	</tr><tr>
		<td></td>
		<td><input type="radio" name="timerange" value="monthly" <?if_monthly?>checked<?/if_monthly?>>Monthly</td>
		<td><input type="radio" name="timerange" value="total" <?if_total?>checked<?/if_total?>>Total</td>
		</td>
	</tr></table>
	</td>
	<td></td>
</tr><!tr>

</tr>
</table>
</form>



<table border="0" cellpadding="1">

<tr align="right">
  <th><?-- "Nr" --?><?slang id="340"/?></th>
  <th align="center"><?-- "User" --?><?slang id="341"/?></th>
  <th align="left"><?-- "Info" --?><?slang id="342"/?></th>
  <th align="left"><?-- "Random Quote" --?><?slang id="343"/?></th>
  <th align="right"><?sorting/?></th>
</tr>

<?toplist?>
<tr>
	<td><?place/?></td>
	<td><a href="../../users/<?user encode="yes"/?>/"><?user/?></a></td>
	<td>
		<?if_icqnr?><img src="http://wwp.icq.com/scripts/online.dll?icq=<?icqnr/?>&amp;img=5" alt="<?icqnr/?>" width="18" height="18"><?/if_icqnr?>
		<?if_binary?>
			<?if_homepage?><a href="<?homepage/?>"><img src="<?binary_url/?>/homepage.gif" height="15" width="15" alt="[URL]" border="0"></a><?/if_homepage?>
			<?if_email?><a href="mailto:<?email/?>"><img src="<?binary_url/?>/email.gif" height="15" width="15" alt="[eMail]" border="0"></a><?/if_email?>
		<?/if_binary?>
	</td>
	<td><?if_quote?><BLOCKQUOTE>&quot;<?random_quote/?>&quot;</BLOCKQUOTE><?/if_quote?></td>
	<td><?value/?></td>
</tr>
<?/toplist?>

</table>




<br>
<?template name="navbar"/?>
<br>

<?template name="credits"/?>
</center>
</body>

</html>
