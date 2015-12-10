class Stack
  def initialize
    @elements = []
  end

  def push(arg)
    @elements << arg
  end

  def pop()
    @elements.pop
  end

end
