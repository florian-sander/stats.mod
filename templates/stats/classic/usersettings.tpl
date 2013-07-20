<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
<title>
  <?-- "Settings for User" --/?><?slang id="1000"/?>
  <?-- CSS --?><?slang id="1"/?>
</title>
</head>

<?-- body tag --?><?slang id="10"/?>
<center>
<H1><?-- "Settings for User" --/?><?slang id="1000"/?></H1><br><br>
<form name="userlogin" ACTION="/cgi-bin/usersettings/" METHOD="<?form_method/?>">
<input type="hidden" name="username" value="<?user/?>">
<input type="hidden" name="password" value="<?password/?>">

<table border="0">
<tr>
  <td><?-- "Age" --?><?slang id="1030"/?>: </td>
  <td><?userage/?></td>
</tr>
<tr>
  <td><?-- "Allowed inactivity" --?><?slang id="1035"/?>:<br>
      <font size="-1"><?-- "(before user gets deleted)" --?>(<?slang id="1036"/?>)</font></td>
  <td><?timetolive/?></td>
</tr>
<tr><td>&nbsp;</td><td>&nbsp;</td></tr>
<tr>
  <td><?-- "ICQ" --/?><?slang id="1050"/?>: </td>
  <td><INPUT TYPE="TEXT" SIZE="10" NAME="icqnr" value="<?icqnr/?>"></td>
</tr>
<tr>
  <td><?-- "eMail" --?><?slang id="1060"/?>: </td>
  <td><INPUT TYPE="TEXT" SIZE="20" MAXLENGTH="100" NAME="email" value="<?email/?>"></td>
</tr>
<tr>
  <td><?-- "Homepage" --/?><?slang id="1070"/?>: </td>
  <td><INPUT TYPE="TEXT" SIZE="30" MAXLENGTH="500" NAME="homepage" value="<?homepage/?>"></td>
</tr>
<tr><td>&nbsp;</td><td>&nbsp;</td></tr>
<tr>
  <td><?-- "List me in the top30?" --/?><?slang id="1080"/?> </td>
  <td>
      <input type="radio" name="list" <?if_listuser?>checked<?/if_listuser?> value="1"><?-- "yes" --/?><?slang id="1085"/?><br>
      <input type="radio" name="list" <?if_not_listuser?>checked<?/if_not_listuser?> value="0"><?-- "no" --/?><?slang id="1086"/?>
  </td>
</tr>
<tr>
  <td><?-- "Automatically add new hosts to my account?" --/?><?slang id="1081"/?> </td>
  <td>
      <input type="radio" name="addhosts" <?if_addhosts?>checked<?/if_addhosts?> value="1"><?-- "yes" --/?><?slang id="1085"/?><br>
      <input type="radio" name="addhosts" <?if_not_addhosts?>checked<?/if_not_addhosts?> value="0"><?-- "no" --/?><?slang id="1086"/?>
  </td>
</tr>
<tr>
  <td><?-- "Publish my stats at all?" --/?><?slang id="1082"/?> <br>
      <font size="-2"><?-- "for those concerned about privacy..." --/?><?slang id="1083"/?></font></td>
  <td>
      <input type="radio" name="nostats" <?if_not_nostats?>checked<?/if_not_nostats?> value="0"><?-- "yes" --/?><?slang id="1085"/?><br>
      <input type="radio" name="nostats" <?if_nostats?>checked<?/if_nostats?> value="1"><?-- "no" --/?><?slang id="1086"/?>
  </td>
</tr>
<tr><td>&nbsp;</td><td>&nbsp;</td></tr>
<tr><td><?slang id="</td><td>&nbsp;</td></tr>
<tr>
  <td><?-- "New Password" --/?><?slang id="1040"/?>: </td>
  <td><INPUT TYPE="password" SIZE="12" MAXLENGTH="1000" NAME="newpassword" value=""></td>
</tr>
<tr>
  <td><?-- "Confirm New Password" --/?><?slang id="1045"/?>: </td>
  <td><INPUT TYPE="password" SIZE="12" MAXLENGTH="1000" NAME="newpass_confirmation" value=""></td>
</tr>
<tr><td>&nbsp;</td><td>&nbsp;</td></tr>
<tr>
  <td><?-- "Change username to" --/?><?slang id="1046"/?>: </td>
  <td><INPUT TYPE="TEXT" SIZE="12" MAXLENGTH="1000" NAME="change_username" value=""></td>
</tr>
<tr>
  <td><?-- "Merge accounts:" --/?><?slang id="1045"/?>: </td>
  <td><INPUT TYPE="password" SIZE="12" MAXLENGTH="1000" NAME="newpass_confirmation" value=""></td>
</tr>
</table>
<br>

<?if_dontpost?><input type="hidden" name="dontpost" value="1"><?/if_dontpost?>

<INPUT TYPE="SUBMIT" VALUE="  Update  ">
</form>

<br>
<?template name="credits"/?>
</body>
</html>