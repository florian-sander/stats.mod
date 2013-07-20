<table border="0">
<caption><?-- "Most used" --?><?slang id="850"/?></caption>
<tr>
  <th><?-- "ISPs" --?><?slang id="855"/?></th>
  <th><?-- "TLDs" --?><?slang id="856"/?></th>
</tr>
<?init_colorfade steps="5"/?>
<?hosts?>
  <tr bgcolor="<?fcolor/?>">
    <td><?ISP/?> (<?ISPnr/?>)</td>
    <td><?TLD/?> (<?TLDnr/?>)</td>
  </tr>
  <?fade_color/?>
<?/hosts?>
</table>