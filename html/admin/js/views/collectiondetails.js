window.CollectionView = Backbone.View.extend({

    initialize: function () {
        this.render();
    },

    render: function () {
        $(this.el).html(this.template(this.model.toJSON()));
        var collName = location.hash.split(",")[1];

        $.ajax({
          type: "GET",
          url: "/_api/collection/" + collName + "/properties" + "?" + getRandomToken(),
          contentType: "application/json",
          processData: false,
          success: function(data) {
            if (data.waitForSync == false) {
              $('#update-collection-sync').val('false');
            }
            else {
              $('#update-collection-sync').val('true');
            }
            $('#update-collection-size').val(data.journalSize);
          },
          error: function(data) {

          }
        });

        return this;
    },

    events: {
        "change"        : "change",
        "click .save"   : "beforeSave",
        "click .delete" : "deleteCollection",
        "drop #picture" : "dropHandler"
    },

    change: function (event) {
      /*
        // Remove any existing alert message
        utils.hideAlert();

        // Apply the change to the model
        var target = event.target;
        var change = {};
        change[target.name] = target.value;
        this.model.set(change);

        // Run validation rule (if any) on changed item
        var check = this.model.validateItem(target.id);
        if (check.isValid === false) {
            utils.addValidationError(target.id, check.message);
        } else {
            utils.removeValidationError(target.id);
        }
        */
    },

    beforeSave: function () {
        /*
        var self = this;
        var check = this.model.validateAll();
        if (check.isValid === false) {
            utils.displayValidationErrors(check.messages);
            return false;
        }
        */
        this.updateCollection();
        return false;
    },

    updateCollection: function () {
      var checkCollectionName = location.hash.split(",")[1];
      var newColName = $('#collectionName').val();
      var currentid = $('#collectionId').val();
      //TODO: CHECK VALUES
      var wfscheck = $('#update-collection-sync').val();
      var journalSize = JSON.parse($('#update-collection-size').val() * 1024 * 1024);
      var wfs = (wfscheck == "true");
      var failed = false;

      if (newColName != checkCollectionName) {
        $.ajax({
          type: "PUT",
          async: false, // sequential calls!
          url: "/_api/collection/" + checkCollectionName + "/rename",
          data: '{"name":"' + newColName + '"}',
          contentType: "application/json",
          processData: false,
          success: function(data) {
            arangoAlert("Collection renamed");
            //TODO: change also local storage collection name
          },
          error: function(data) {
            alert(getErrorMessage(data));
            failed = true;
          }
        });
      }
      if (! failed) {
        $.ajax({
          type: "PUT",
          async: false, // sequential calls!
          url: "/_api/collection/" + newColName + "/properties",
          data: '{"waitForSync":' + (wfs ? "true" : "false") + ',"journalSize":' + JSON.stringify(journalSize) + '}',
          contentType: "application/json",
          processData: false,
          success: function(data) {
            arangoAlert("Saved collection properties");
          },
          error: function(data) {
            alert(getErrorMessage(data));
            failed = true;
          }
        });
      }
      if (! failed) {
        window.history.back();
      }
      else {
        return 0;
      }
    },

    deleteCollection: function () {
        var self = this;
        $.ajax({
          type: 'DELETE',
          url: "/_api/collection/" + self.model.attributes.id,
          success: function () {
            self.model.destroy({
              success: function () {
                window.history.back();
              }
            });
          },
          error: function () {
            alert('Error');
          }
        });
    },

    dropHandler: function (event) {
        event.stopPropagation();
        event.preventDefault();
        var e = event.originalEvent;
        e.dataTransfer.dropEffect = 'copy';
        this.pictureFile = e.dataTransfer.files[0];

        // Read the image file from the local file system and display it in the img tag
        var reader = new FileReader();
        reader.onloadend = function () {
            $('#picture').attr('src', reader.result);
        };
        reader.readAsDataURL(this.pictureFile);
    }

});
