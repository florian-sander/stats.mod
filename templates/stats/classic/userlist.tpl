<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
  <META NAME="ROBOTS" CONTENT="INDEX, NOFOLLOW">
  <?-- CSS --?><?slang id="1"/?>
  <title><?slang id="500"/?></title>
</head>


<?-- body tag --?><?slang id="10"/?>
<center>
<table border="0">
<tr><th>User</th></tr>
<?init_colorfade steps="users"/?>
<?userlist?>
<tr bgcolor="<?fcolor/?>"><td><a href="<?user encode="yes"/?>/"><?user/?></a></td></tr><?fade_color/?>
<?/userlist?>
</table>
<br>
<br>
<?template name="navbar"/?>
<br>
<?template name="credits"/?>
</center>
</body>

</html>