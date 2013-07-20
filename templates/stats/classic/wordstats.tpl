<?-- "Registered users spoke 1185 different words" --?><?slang id="870"/?><br>
<?-- "These are the most used ones:" --?><?slang id="875"/?><br>
<table border="0">
<?init_colorfade steps="20"/?>
<?chan_topwords?>
  <tr>
    <td><?wordplace/?></td>
    <td bgcolor="<?fcolor/?>"><?word/?></td>
    <?fade_color/?>
    <td bgcolor="<?fcolor/?>"><font size="-1">(<?wordnr/?>)</font></td>
    <?fade_color/?>
  </tr>
<?/chan_topwords?>
</table>