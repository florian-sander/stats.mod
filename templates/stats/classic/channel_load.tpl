<?-- "average users" --?><?slang id="820"/?><br>

<?if_binary?>
<table border="0">
<tr>
  <td></td>
  <td colspan="6" align="center"><font size="-5">0:00 - 6:00</font></td>
  <td colspan="6" align="center"><font size="-5">6:00 - 12:00</font></td>
  <td colspan="6" align="center"><font size="-5">12:00 - 17:00</font></td>
  <td colspan="6" align="center"><font size="-5">18:00 - 24:00</font></td>
</tr>
<tr>
  <td></td>
  <?channel_load?>
  <td align="center" valign="bottom">
    <?if_cl_logged?>
      <img src="<?binary_url/?>/vertical_blue_bar.gif" height="<?AU_value max="100"/?>" width="10" alt="blue">
      <img src="<?binary_url/?>/vertical_green_bar.gif" height="<?activity_value max="100"/?>" width="10" alt="green">
    <?/if_cl_logged?>
  </td>
  <?/channel_load?>
</tr>
<tr>
  <td><font color="#0E00BF" size="-1"><?slang id="825"/?></font></td>
  <?channel_load?>
  <td align="center">
    <?if_cl_logged?>
      <font size="-5"><?AU_users/?></font>
    <?/if_cl_logged?>
  </td>
  <?/channel_load?>
</tr>
<tr>
  <td><font color="#1F7E58" size="-1"><?slang id="826"/?></font></td>
  <?channel_load?>
  <td align="center">
    <?if_cl_logged?>
      <font size="-5"><?activity/?></font>
    <?/if_cl_logged?>
  </td>
  <?/channel_load?>
</tr>
</table>
<?/if_binary?>