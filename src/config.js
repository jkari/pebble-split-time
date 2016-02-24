var MESSAGE_TYPE_CONFIG = 3;

Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL('http://95.85.12.164/splittime/');
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode and parse config data as JSON
  var config_data = JSON.parse(decodeURIComponent(e.response));
  console.log('Config window returned: ', JSON.stringify(config_data));

  // Send settings to Pebble watchapp
  Pebble.sendAppMessage(
    {
      'KEY_MESSAGE_TYPE': MESSAGE_TYPE_CONFIG,
      'KEY_USE_CELCIUS': parseInt(config_data['use_celcius']),
      'KEY_COLOR_LEFT': parseInt(config_data['color_left'].substr(config_data['color_left'].length - 6), 16),
      'KEY_COLOR_RIGHT': parseInt(config_data['color_right'].substr(config_data['color_right'].length - 6), 16)
    },
    function(){
      console.log('Sent config data to Pebble');  
    },
    function() {
      console.log('Failed to send config data!');
    }
  );
});