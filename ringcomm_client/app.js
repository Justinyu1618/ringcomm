const express = require("express");
const app = express();
const port = process.env.PORT || 8000;

app.use(express.static("public"));

console.log(process.env.TWILIO_ACCOUNT_SID);
const accountSid = process.env.TWILIO_ACCOUNT_SID;
const authToken = process.env.TWILIO_AUTH_TOKEN;
const client = require("twilio")(accountSid, authToken);

app.get("/send-sms", (req, res) => {
  const msg = req.query.msg;
  const number = req.query.number || "+16179570655";

  client.messages
    .create({
      body: msg,
      from: "+16173624067",
      to: number,
    })
    .then((message) => console.log(message.sid));
});

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`);
});
