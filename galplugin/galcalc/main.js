(function calcExpression(expression) {
  try {
    return eval(expression);
  }
  catch(err) {
    return "Invalid Expression"
  }
})