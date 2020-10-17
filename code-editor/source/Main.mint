component Main {
  style base {
    display: flex;
    flex-direction: column;
    height: 100vh;

    .CodeMirror {
      height: 100%;
    }
  }

  style editor {
    width: 100vw;
    height: 80vh;
  }

  style terminal {
    background-color: black;
    height: 20vh;
    overflow: hidden !important;

    div.xterm-viewport {
      width: 100%;
      overflow: hidden !important;
    }
  }

  fun componentDidMount {
    sequence {
      e =
        case (editor) {
          Maybe::Just element => element
          Maybe::Nothing => Dom.createElement("div")
        }

      t =
        case (terminal) {
          Maybe::Just element => element
          Maybe::Nothing => Dom.createElement("div")
        }

      `
      function(){
        window.addEventListener('DOMContentLoaded', (event) => {
          window.editor = CodeMirror(#{e},{
            mode:"ruby",
            theme:"dracula",
            lineNumbers: true
          });
          window.mruby = Module.cwrap("run",null,["string"]);
          window.terminal = new Terminal();
          window.terminal.open(#{t});
          fit.fit(window.terminal);
          window.out = function(o){
            window.terminal.write(o);
            window.terminal.write("\n\r");
          }
          window.err = function(e){
            window.terminal.write(e);
            window.terminal.write("\n\r");
          }
        });
      }()
      `
    }
  }

  fun render : Html {
    <div::base>
      <div::editor as editor/>
      <div::terminal as terminal/>
      <FloatingButtons/>
    </div>
  }
}
