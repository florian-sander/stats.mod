<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
  <title><?-- "User Login" --/?><?slang id="1000"/?></title>
  <?-- CSS --?><?slang id="1"/?>
</head>


<?-- body tag --?><?slang id="10"/?>

<br>
<center>
<form ACTION="/cgi-bin/usersettings/" METHOD="<?form_method/?>">

<?-- "Username" --/?><?slang id="1010"/?>: <INPUT TYPE="TEXT" SIZE="12" MAXLENGTH="20" NAME="username" value="<?user/?>"><br>
<?-- "Password" --/?><?slang id="1020"/?>: <INPUT TYPE="password" SIZE="12" MAXLENGTH="1000" NAME="password"><br>
<br>
<font size="-1"><?slang id="1030"/?><br>
<kbd>&quot;/msg <?botnick/?> STATSPASS &lt;password&gt;&quot;</kbd><br>
<?slang id="1040"/?></font><br>

<?if_dontpost?><input type="hidden" name="dontpost" value="1"><?/if_dontpost?>

<INPUT TYPE="SUBMIT" VALUE="  <?-- "Login" --/?><?slang id="1050"/?>  "><INPUT TYPE="SUBMIT" VALUE="<?-- "I forgot my password" --/?><?slang id="1060"/?>" name="sendpass">
</form>

</center>
<br>
<?template name="credits"/?>
</body>

</html>