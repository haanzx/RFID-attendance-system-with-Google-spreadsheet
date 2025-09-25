var SHEET_NAME = "Sheet4"; 
var TIME_ZONE = "Asia/Jakarta"; // Zona waktu Indonesia Barat

function doGet(e) {
  var data = e.parameter;
  var uid = data.id;
  var status = data.status; 
  var now = new Date(); 
  
  // Format Tanggal dan Waktu (digunakan untuk mencatat & mencari)
  var dateString = Utilities.formatDate(now, TIME_ZONE, "yyyy-MM-dd");
  var timeString = Utilities.formatDate(now, TIME_ZONE, "HH:mm:ss");

  if (!uid || !status) {
    return ContentService.createTextOutput("Error: UID atau Status tidak terkirim.");
  }
  
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName(SHEET_NAME);
  if (!sheet) {
    return ContentService.createTextOutput("Error: Sheet4 tidak ditemukan.");
  }
  
  var dataRange = sheet.getDataRange();
  var values = dataRange.getValues();
  
  // Indeks Kolom (di Apps Script, indeks dimulai dari 0)
  var UID_COLUMN_INDEX = 0;    // Kolom A
  var DATE_COLUMN_INDEX = 1;   // Kolom B
  var DATANG_COLUMN_INDEX = 2; // Kolom C
  var PULANG_COLUMN_INDEX = 3; // Kolom D

  // Mulai dari baris kedua (indeks 1) untuk melewati header
  for (var i = 1; i < values.length; i++) { 
    var rowUID = values[i][UID_COLUMN_INDEX]; 
    var rowDate = values[i][DATE_COLUMN_INDEX]; 

    // Mencari baris yang cocok (UID DAN Tanggal hari ini)
    if (rowUID === uid && rowDate === dateString) {
      
      // Jika status 'Masuk', perbarui Kolom C (Waktu Datang)
      if (status === 'Masuk') {
        sheet.getRange(i + 1, DATANG_COLUMN_INDEX + 1).setValue(timeString);
      } 
      // Jika status 'Pulang', perbarui Kolom D (Waktu Pulang)
      else if (status === 'Pulang') {
        sheet.getRange(i + 1, PULANG_COLUMN_INDEX + 1).setValue(timeString);
      }
      
      return ContentService.createTextOutput("Data Berhasil Diperbarui: " + status);
    }
  }

  // Jika UID tidak ditemukan pada tanggal hari ini, tambahkan baris baru
  var newRowData;
    
  if (status === 'Masuk') {
    // [UID, Tanggal, Waktu Datang, Waktu Pulang]
    newRowData = [uid, dateString, timeString, '']; 
  } else {
    // Jika Pulang duluan, Masuk dikosongkan
    newRowData = [uid, dateString, '', timeString]; 
  }

  sheet.appendRow(newRowData);
  return ContentService.createTextOutput("Data Baru Berhasil Ditambahkan: " + status);
}
