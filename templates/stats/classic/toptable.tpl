<table border="0" cellpadding="1">

<tr align="right">
  <th><?-- "Nr" --?><?slang id="340"/?></th>
  <th align="center"><?-- "User" --?><?slang id="341"/?></th>
  <th align="left"><?-- "Info" --?><?slang id="342"/?></th>
  <?topstats?>
    <th><a href="../<?type/?>/"><?type slang="yes"/?></a></th>
  <?/topstats?>
</tr>

<?init_colorfade steps="toplist"/?>

<?toplist?>
  <tr align="right" bgcolor="<?fcolor/?>">
    <td><?place/?></td>
    <td>
      <a href="../../../users/<?user encode="no"/?>/"><?user/?></a>
    </td>
    <td>
      <?if_icqnr?><img src="http://wwp.icq.com/scripts/online.dll?icq=<?icqnr/?>&amp;img=5" alt="<?icqnr/?>" width="18" height="18"><?/if_icqnr?>
	  <?if_binary?>
	    <?if_homepage?><a href="<?homepage/?>"><img src="<?binary_url/?>/homepage.gif" height="15" width="15" alt="[URL]" border="0"></a><?/if_homepage?>
	    <?if_email?><a href="mailto:<?email/?>"><img src="<?binary_url/?>/email.gif" height="15" width="15" alt="[eMail]" border="0"></a><?/if_email?>
	  <?/if_binary?>
    </td>
    <?topstats?>
      <td><?value/?></td>
    <?/topstats?>
  </tr>
  <?fade_color/?>
<?/toplist?>

</table>
