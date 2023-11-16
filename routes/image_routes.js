const express = require('express');
// const client = require('../db/db.js');
const controller =  require('../controllers/upload.js');

module.exports = function(app) {
  //route to upload single image  
  app.post('/upload/upload-single-image', controller.upload.single('imageFile'), controller.uploadSingleImage);
};
