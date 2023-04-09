import { Box, Button, FileInput, Heading, Layer } from "grommet";

export function SFDLModal(props: { close: () => void }) {
  const { close } = props;

  return (
    <Layer onEsc={close} onClickOutside={close}>
      <Box direction="column" border={{ color: "brand" }} pad="large">
        <Heading size={"small"}>Add SFDL-File</Heading>
        <FileInput
          name="file"
          onChange={(event) => {
            if (event?.target.files) {
              const fileList = event.target.files;
              for (let i = 0; i < fileList.length; i += 1) {
                const file = fileList[i];
              }
            }
          }}
        />
        <Button primary label="Close" onClick={close} />
      </Box>
    </Layer>
  );
}
