<?php

function put_image($file, $alt_text, $extras='border="0"')
{
   $size = getimagesize($file);

   echo '<img width="'.$size[0].'" height="'.$size[1].'" alt="'.$alt_text.'" src="'.$file.'" '.$extras.'>';
}

$file = 'screenshots/' . $shot . '.gif';
$desc = 'screenshots/' . $shot . '.desc';

if ((isset($shot)) && (file_exists($file)) && (file_exists($desc)))
{
	// Header the page
	$title = 'Screenshots: '.ucfirst($shot);
	include('layout/header');

	// Include the image
	echo '<p align="center">';
	put_image($file, '');
	echo '</p>';

	// Include the text
	echo '<p align="center">';
	include($desc);
	echo '</p>';
}
else
{
	$title = 'Screenshot not found';
	include('layout/header');
}
?>


<?php include('layout/footer'); ?>
