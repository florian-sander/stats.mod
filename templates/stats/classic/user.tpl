<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
  <?-- CSS --?><?slang id="1"/?>
  <title><?slang id="700"/?></title>
</head>


<?-- body tag --?><?slang id="10"/?>
<center>
<H1><?-- title --?><?slang id="710"/?></H1><br>

<table border="0">
<?if_icqnr?>
  <tr>
    <td align="right">
      <img src="http://wwp.icq.com/scripts/online.dll?icq=<?icqnr/?>&amp;img=5" width="14" height="14" alt="<?icqnr/?>">
      <?-- "ICQ#" --?><?slang id="720"/?>
    </td>
    <td align="left">&nbsp;&nbsp;<?icqnr/?></td>
  </tr>
<?/if_icqnr?>
<?if_email?>
  <tr>
    <td align="right"><?-- "eMail" --?><?slang id="721"/?></td>
    <td>&nbsp;&nbsp;<a href="mailto:<?email/?>"><?email/?></a></td>
  </tr>
<?/if_email?>
<?if_homepage?>
  <tr>
    <td align="right"><?-- "Homepage" --?><?slang id="722"/?></td>
    <td>&nbsp;&nbsp;<a href="<?homepage/?>" target="_blank"><?homepage/?></a></td>
  </tr>
<?/if_homepage?>
  <tr>
    <td align="right"><?-- "Age" --?><?slang id="723"/?></td>
    <td>&nbsp;&nbsp;<?memberage relative="no"/?></td>
  </tr>
</table>

<br>
<br>

<table border="0">
  <tr>
    <td></td>
    <td><?-- "place" --?><?slang id="730"/?></td>
    <?topstats?>
      <td><?type slang="yes"/?></td>
    <?/topstats?>
  </tr>
  <?init_colorfade steps="8"/?>
  <?timeranges?>
  <tr>
    <td align="center">
      <?if_total?><?-- "total" --?><?slang id="760"/?><?/if_total?>
      <?if_daily?><?-- "today" --?><?slang id="761"/?><?/if_daily?>
      <?if_weekly?><?-- "this week" --?><?slang id="762"/?><?/if_weekly?>
      <?if_monthly?><?-- "this month" --?><?slang id="763"/?><?/if_monthly?>
    </td>
    <td align="right" bgcolor="<?fcolor/?>"><?tplace/?></td>
    <?fade_color/?>
    <?topstats?><td bgcolor="<?fcolor/?>" align="right"><?value/?></td><?/topstats?>
    <?fade_color/?>
  </tr>
  <?/timeranges?>
</table>

<?if_quote?><?-- "random quote" --?><?slang id="740"/?>&quot;<?random_quote/?>&quot;<br><?/if_quote?>
<br>
<?if_user_topwords?>
  <?-- "_Troll_ spoke 148 different words today. These are the most used ones:" --?><?slang id="750"/?><br>
  <table border="0">
  <?init_colorfade steps="20"/?>
  <?user_topwords?>
    <tr>
      <td><?wordplace/?></td>
      <td bgcolor="<?fcolor/?>"><?word/?></td>
      <?fade_color/?>
      <td bgcolor="<?fcolor/?>"><font size="-1">(<?wordnr/?>)</font></td>
      <?fade_color/?>
    </tr>
  <?/user_topwords?>
  </table>
<?/if_user_topwords?>
<br>
<?template name="navbar"/?>
<br>
<font size="-2"><a href="/cgi-bin/usersettings/?username=<?user encode="yes"/?>"><?-- "edit my Settings" --/?><?slang id="780"/?></a></font><br>
<br>
<?template name="credits"/?>
</center>
</body>

</html>
