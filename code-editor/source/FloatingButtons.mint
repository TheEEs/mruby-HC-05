component FloatingButtons {
  state fileOpen = false

  state selectedFile = ""

  state files = []

  state fileFilter : String = ""

  style icon {
    display: flex;
    bottom: 0px;
    top: 0px;
    justify-content: center;
    align-items: center;

    img,
    svg {
      height: 50%;
      display: flex;
    }

    i.ionicon {
      font-size: 40px;
    }
  }

  style modal {
    if (!fileOpen) {
      display: none;
    }

    .is-vcentered .column {
      display: flex;
      align-items: center;
      justify-content: flex-end;
    }

    max-height: 200px !important;
  }

  fun compileAndRun {
    `
        function(){
            window.terminal.write("\n\r");
            window.terminal.clear();
            mruby(window.editor.getValue());
        }()
        `
  }

  fun openFile {
    sequence {
      xfiles =
        getFiles()

      next { files = xfiles }
      next { fileOpen = true }
      next { selectedFile = "" }

      value =
        `swal({
              content : #{modalx}._0,
              button: "Open"
            })`

      fileContent =
        `
            function(path){
              if(path && typeof(path) === "string"){
                try{
                  return Capacitor.Plugins.Filesystem.readFile({
                    path,
                    directory: "EXTERNAL",
                    encoding: "utf8"
                  }).then(d => d.data).then((code)=>{
                    window.editor.setValue(code);
                  }).catch(() => "");
                }catch(e){
                  console.log(e);
                  return "";
                }
              }
            }(#{value})
            `
    }
  }

  fun saveFile {
    sequence {
      fileData =
        `window.editor.getValue()`

      filePath =
        `
          swal({
            content: {
              element: "input",
              attributes: {
                placeholder: "File name",
              },
            },text: "Save File"
          }).then(result => result ? result : "")
        `

      `
        function(path,data){
          if(path)
            return Capacitor.Plugins.Filesystem.writeFile({
              path,
              directory: "EXTERNAL",
              encoding: "utf8", 
              data}).then(()=>{
                return Capacitor.Plugins.Toast.show({
                  text: "File saved"
                })
              }).catch(()=>{
                return Capacitor.Plugins.Toast.show({
                  text: "Cannot save file"
                })
              })
          else 
            return;
        }(#{filePath},#{fileData})
        `
    }
  }

  fun reloadWindow(){
    `window.location.reload()`
  }

  fun getFiles : Promise(Never, Array(String)) {
    sequence {
      unDecodedFiles =
        `
          function(){
            try{
              return Capacitor.Plugins.Filesystem.readdir({path: "/",directory: "EXTERNAL"}).then(res => res.files);
            }catch{
              return ["a","b","c"];
            }
          }()
        `

      decodedFiles =
        decode unDecodedFiles as Array(String)

      decodedFiles
      |> Array.sortBy((file : String) : String { file })
    } catch Object.Error => error {
      []
    }
  }

  fun selectFile (e : Html.Event) {
    sequence {
      target =
        e.currentTarget

      Html.Event.stopPropagation(e)

      fileName =
        (target
        |> Dom.getAttribute("data-file-name"))

      encodedFileName =
        encode fileName

      `swal.setActionValue(#{encodedFileName})`

      case (fileName) {
        Maybe::Just file => next { selectedFile = file }
        Maybe::Nothing => next { selectedFile = "" }
      }
    }
  }

  fun isActive (file : String) {
    if (file == selectedFile) {
      "is-active"
    } else {
      ""
    }
  }

  fun deleteFile (path : String, e : Html.Event) {
    sequence {
      Html.Event.stopPropagation(e)

      encodedPath =
        encode path

      undecodedResult =
        `
          function(path){
            try{
              return swal("You wanna delete this file?",{
                buttons: ["No", "Yes"],
                dangerMode: true
              }).then(function(e){
                if(e){
                  return Capacitor.Plugins.Filesystem.deleteFile({
                    path,
                    directory: "EXTERNAL"
                  }).then(()=> {
                    return Capacitor.Plugins.Toast.show({
                      text: "File deleted"
                    })
                  }).catch(()=> {
                  return Capacitor.Plugins.Toast.show({
                      text: "Cannot delete this file"
                    }) 
                  })   
                }
              });
            }catch(e){
              console.log(e);
              return 0;
            }
          }(#{encodedPath})
        `
    }
  }

  fun filterChange (e : Html.Event) {
    try {
      case (fileNameFilter) {
        Maybe::Just element =>
          sequence {
            next { fileFilter = Dom.getValue(element) }
          }

        Maybe::Nothing => Promise.never()
      }
    }
  }

  fun modal : Html {
    <nav::modal as modalx
      class="panel"
      id="select_file_modal">

      <p class="panel-heading">
        "Open File"
      </p>

      <div class="panel-block">
        <p class="control has-icons-left">
          <input as fileNameFilter
            onChange={filterChange}
            class="input"
            type="text"
            placeholder="Search"/>

          <span class="icon is-left">
            <ion-icon
              size="small"
              name="search-outline"/>
          </span>
        </p>
      </div>

      for (file of files
      |> Array.select(
        (file : String) : Bool {
          fileFilter == "" || (file |> String.match(fileFilter))
        })) {
        <a
          class={"panel-block " + isActive(file)}
          onClick={selectFile}
          data-file-name={file}>

          <span class="panel-icon">
            <ion-icon
              size="medium"
              name="document-outline"/>
          </span>

          <{ file }>

          <div class="block container is-vcentered columns">
            <span class="column is-offset-10 is-2">
              <ion-icon
                onClick={deleteFile(file)}
                name="trash-outline"/>
            </span>
          </div>

        </a>
      }

    </nav>
  }

  fun render : Html {
    <ul
      class="mfb-component--br mfb-slidein"
      data-mfb-toggle="hover">

      <{ modal() }>

      <li class="mfb-component__wrap">
        <a
          href="#"
          data-mfb-label="Menu"
          class="mfb-component__button--main">

          <i::icon class="mfb-component__child-icon ion-paper-airplane">
            <ion-icon
              size="large"
              name="ellipsis-horizontal-circle-outline"/>
          </i>

        </a>

        <ul class="mfb-component__list">
          <li>
            <a
              href="#"
              onClick={compileAndRun}
              data-mfb-label="Run"
              class="mfb-component__button--child">

              <i::icon class="mfb-component__child-icon ion-paper-airplane">
                <ion-icon
                  size="large"
                  name="play-circle-outline"/>
              </i>

            </a>
          </li>

          <li>
            <a
              href="#"
              onClick={reloadWindow}
              data-mfb-label="Reload the app"
              class="mfb-component__button--child">

              <i::icon class="mfb-component__child-icon ion-paper-airplane">
                <ion-icon size="large" name="refresh-outline"></ion-icon>
              </i>

            </a>
          </li>

          <li>
            <a
              href="#"
              data-mfb-label="Open"
              onClick={openFile}
              class="mfb-component__button--child">

              <i::icon class="mfb-component__child-icon ion-paper-airplane">
                <ion-icon
                  name="folder-open-outline"
                  size="large"/>
              </i>

            </a>
          </li>

          <li>
            <a
              href="#"
              data-mfb-label="Save"
              onClick={saveFile}
              class="mfb-component__button--child">

              <i::icon class="mfb-component__child-icon ion-paper-airplane">
                <ion-icon
                  size="large"
                  name="code-download-outline"/>
              </i>

            </a>
          </li>
        </ul>
      </li>

    </ul>
  }
}
