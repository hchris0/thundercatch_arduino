using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Auth;
using Microsoft.WindowsAzure.Storage.Table;
using System.Data;

namespace TestSiteExample
{
    public partial class tabletest : System.Web.UI.Page
    {
        CloudStorageAccount account = new CloudStorageAccount(
    new StorageCredentials([Storage account name],
        [Storage account key]), true); 

        protected void Page_Load(object sender, EventArgs e)
        {
            fillGrid(readTable(true));
        }

        public class Lightnings : TableEntity
        {
            public Lightnings() { }
            public string Distance { get; set; }
            public string Event { get; set; }
            public string Device { get; set; }
        } 

        public void fillGrid(IEnumerable<Lightnings> data)
        {

            DataTable dt = new DataTable();
            DataRow dr = null;
            dt.Columns.Add(new DataColumn("Device ID", typeof(string)));
            dt.Columns.Add(new DataColumn("Event Type", typeof(string)));
            dt.Columns.Add(new DataColumn("Distance (km)", typeof(string)));
            dt.Columns.Add(new DataColumn("Date", typeof(string)));
            dt.Columns.Add(new DataColumn("Time", typeof(string)));

            if (!data.Any<Lightnings>()) {
                dr = dt.NewRow();
                dr["Device ID"] = string.Empty;
                dr["Event Type"] = string.Empty;
                dr["Distance (km)"] = string.Empty;
                dr["Date"] = string.Empty;
                dr["Time"] = string.Empty;
                dt.Rows.Add(dr);
            } else {
                // Print the fields for each customer. 
                foreach (Lightnings entity in data) {
                    DateTimeOffset dto = new DateTimeOffset();
                    dto = entity.Timestamp;
                    string eventType = entity.Event;
                    if (eventType == "1") {
                        eventType = "Noise";
                    } else if (eventType == "2") {
                        eventType = "Disturber";
                    } else if (eventType == "3") {
                        eventType = "Lightning";
                    }
                    dr = dt.NewRow();
                    dr["Device ID"] = entity.Device;
                    dr["Event Type"] = eventType;
                    dr["Distance (km)"] = entity.Distance;
                    dr["Date"] = dto.Day.ToString("D2") + "-" + dto.Month.ToString("D2") + "-" + dto.Year.ToString();
                    dr["Time"] = dto.Hour.ToString("D2") + ":" + dto.Minute.ToString("D2") + ":" + dto.Second.ToString("D2");
                    dt.Rows.Add(dr);
                }
            }
            //Store the DataTable in ViewState
            ViewState["CurrentTable"] = dt;

            gridRawData.DataSource = dt;
            gridRawData.DataBind();
        }

        public IEnumerable<Lightnings> readTable(bool readAllTable)
        {
            // Create the table client 
            CloudTableClient tableClient = account.CreateCloudTableClient();
            // Create the CloudTable object that represents the table
            CloudTable table = tableClient.GetTableReference([Storage table name]);
            // Query filter
            string filter = "(PartitionKey eq //PartitionKey//)";
            // Construct the query operation 
            TableQuery<Lightnings> query = new TableQuery<Lightnings>().Where(filter);
            // Get data from table
            IEnumerable<Lightnings> data = table.ExecuteQuery(query);
            var sortedData = data.OrderByDescending(c => c.Timestamp);
            return sortedData;
        }

    }
}