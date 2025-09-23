var SHEET_NAME = "Sheet4"; 

function doGet(e) {
  var data = e.parameter;
  var uid = data.id;
  var status = data.status;
  var now = new Date(); 

  if (uid && status) {
    var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName(SHEET_NAME);
    
    var rowData = [
      uid,
      status,
      now.toLocaleDateString(),
      now.toLocaleTimeString()
    ];
    
    sheet.appendRow(rowData);
    
    return ContentService.createTextOutput("Data berhasil ditambahkan!");
  }
}
