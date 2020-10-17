class Fixnum
  def pin 
    Pin.new self
  end
end

class Pin 
  class Exception < ::Exception 
  end
  def set_mode(mode) 
    case mode 
    when :input,:output
      @mode = mode 
    else 
      raise Exception.new "invalid mode"
    end
  end

  def mode 
    @mode 
  end
end